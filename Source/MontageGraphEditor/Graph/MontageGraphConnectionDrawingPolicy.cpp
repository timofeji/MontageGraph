#include "MontageGraphConnectionDrawingPolicy.h"

#include "MontageGraphEdGraph.h"
#include "MontageGraphEditorStyles.h"
#include "MontageGraphDebugger.h"
#include "Slate/SMontageGraphSelectorOutputPin.h"
#include "Nodes/MontageGraphEdNode.h"
#include "Nodes\MontageGraphEdNodeEntry.h"

FMontageGraphConnectionDrawingPolicy::FMontageGraphConnectionDrawingPolicy(
	const int32 InBackLayerID, const int32 InFrontLayerID, const float ZoomFactor, const FSlateRect& InClippingRect,
	FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj)
	: FConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, ZoomFactor, InClippingRect, InDrawElements)
	  , GraphObj(InGraphObj)
{
	HBActionEdGraph = Cast<UMontageGraphEdGraph>(GraphObj);
}

void FMontageGraphConnectionDrawingPolicy::DetermineWiringStyle(UEdGraphPin* OutputPin, UEdGraphPin* InputPin,
                                                                FConnectionParams& Params)
{
	Params.AssociatedPin1 = OutputPin;
	Params.AssociatedPin2 = InputPin;
	Params.WireThickness = 1.5f;


	if (HBActionEdGraph && InputPin)
	{
		if (auto HBEdNode{Cast<UMontageGraphEdNode>(InputPin->GetOwningNode())})
		{
			if (HBActionEdGraph->Debugger.Get()->SelectedNodesDebug.Contains(HBEdNode->RuntimeNode))
			{
				Params.WireColor = FLinearColor::Red;
				Params.bDrawBubbles = true;
			}
		}
	}

	if (HoveredPins.Num() > 0)
	{
		ApplyHoverDeemphasis(OutputPin, InputPin, Params.WireThickness, Params.WireColor);
	}
}

void FMontageGraphConnectionDrawingPolicy::Draw(TMap<TSharedRef<SWidget>, FArrangedWidget>& InPinGeometries,
                                                FArrangedChildren& ArrangedNodes)
{
	// Build an acceleration structure to quickly find geometry for the nodes
	NodeWidgetMap.Empty();
	for (int32 NodeIndex = 0; NodeIndex < ArrangedNodes.Num(); ++NodeIndex)
	{
		FArrangedWidget& CurWidget = ArrangedNodes[NodeIndex];
		const TSharedRef<SGraphNode> ChildNode = StaticCastSharedRef<SGraphNode>(CurWidget.Widget);
		NodeWidgetMap.Add(ChildNode->GetNodeObj(), NodeIndex);
	}

	// Now draw
	FConnectionDrawingPolicy::Draw(InPinGeometries, ArrangedNodes);
}

void FMontageGraphConnectionDrawingPolicy::DrawPreviewConnector(const FGeometry& PinGeometry,
                                                                const FVector2D& StartPoint, const FVector2D& EndPoint,
                                                                UEdGraphPin* Pin)
{
	FConnectionParams Params;
	DetermineWiringStyle(Pin, nullptr, /*inout*/ Params);

	if (Pin->Direction == EGPD_Output)
	{
		Params.bDrawBubbles = true;
		DrawSplineWithArrow(FGeometryHelper::FindClosestPointOnGeom(PinGeometry, EndPoint), EndPoint, Params);
	}
	else
	{
		DrawSplineWithArrow(FGeometryHelper::FindClosestPointOnGeom(PinGeometry, StartPoint), StartPoint, Params);
	}
}

void FMontageGraphConnectionDrawingPolicy::DrawSplineWithArrow(const FVector2D& StartPoint, const FVector2D& EndPoint,
                                                               const FConnectionParams& Params)
{
	// bUserFlag1 indicates that we need to reverse the direction of connection (used by debugger)
	const FVector2D& P0 = Params.bUserFlag1 ? EndPoint : StartPoint;
	const FVector2D& P1 = Params.bUserFlag1 ? StartPoint : EndPoint;

	Internal_DrawLineWithArrow(P0, P1, Params);
}

void FMontageGraphConnectionDrawingPolicy::Internal_DrawLineWithArrow(const FVector2D& StartAnchorPoint,
                                                                      const FVector2D& EndAnchorPoint,
                                                                      const FConnectionParams& Params)
{
	//@TODO: Should this be scaled by zoom factor?
	constexpr float LineSeparationAmount = 6.5f;

	const FVector2D DeltaPos = EndAnchorPoint - StartAnchorPoint;
	const FVector2D UnitDelta = DeltaPos.GetSafeNormal();
	const FVector2D Normal = FVector2D(DeltaPos.Y, -DeltaPos.X).GetSafeNormal();

	// Come up with the final start/end points
	const FVector2D DirectionBias = Normal * LineSeparationAmount;
	const FVector2D LengthBias = ArrowRadius.X * UnitDelta;
	const FVector2D StartPoint = StartAnchorPoint + DirectionBias + LengthBias;
	const FVector2D EndPoint = EndAnchorPoint + DirectionBias - LengthBias;
	const FVector2D Mid = DeltaPos / LineSeparationAmount;

	const FVector2D P2 = StartPoint + ((Normal.X < Normal.Y)
		                                   ? FVector2D(0.f, Mid.Y)
		                                   : FVector2D(Mid.X, 0.f));


	const FVector2D P3 = EndPoint - ((Normal.X < Normal.Y)
		                                 ? FVector2D(0.f, Mid.Y)
		                                 : FVector2D(Mid.X, 0.f));


	// Draw a line/spline
	// DrawConnection(WireLayerID, StartPoint, EndPoint, Params);
	DrawConnection(WireLayerID, StartPoint, P2, Params);
	DrawConnection(WireLayerID, P2, P3, Params);
	DrawConnection(WireLayerID, P3, EndPoint, Params);


	FSlateFontInfo FontInfo = FMontageGraphEditorStyles::Get().GetFontStyle("HB.Font.Small");
	FontInfo.OutlineSettings.OutlineSize = 1;
	FontInfo.OutlineSettings.OutlineColor = FLinearColor::Black;
	FontInfo.Size = 19;

	if (auto EntryPin = Cast<UMontageGraphEdNodeEntry>(Params.AssociatedPin1->GetOwningNode()))
	{
		FPaintGeometry TextGeometry = FPaintGeometry(P2, FVector2D(50.f, 50.f) * ZoomFactor, ZoomFactor * 0.5f);
		FSlateDrawElement::MakeText(
			DrawElementsList,
			ArrowLayerID + 1,
			TextGeometry,
			Params.AssociatedPin1->PinName.ToString(),
			FontInfo,
			ESlateDrawEffect::None,
			Params.WireColor.Desaturate(0.5f));
	}


	// Draw the arrow
	const FVector2D ArrowDrawPos = EndPoint - ArrowRadius;
	const float AngleInRadians = static_cast<float>(FMath::Atan2(DeltaPos.Y, DeltaPos.X));

	FSlateDrawElement::MakeRotatedBox(
		DrawElementsList,
		ArrowLayerID,
		FPaintGeometry(ArrowDrawPos, ArrowImage->ImageSize * ZoomFactor, ZoomFactor),
		ArrowImage,
		ESlateDrawEffect::None,
		AngleInRadians,
		TOptional<FVector2D>(),
		FSlateDrawElement::RelativeToElement,
		Params.WireColor
	);
}

void FMontageGraphConnectionDrawingPolicy::DrawSplineWithArrow(const FGeometry& StartGeom, const FGeometry& EndGeom,
                                                               const FConnectionParams& Params)
{
	// Get a reasonable seed point (halfway between the boxes)
	const FVector2D StartCenter = FGeometryHelper::CenterOf(StartGeom);
	const FVector2D EndCenter = FGeometryHelper::CenterOf(EndGeom);
	const FVector2D SeedPoint = (StartCenter + EndCenter) * 0.5;

	// Find the (approximate) closest points between the two boxes
	const FVector2D StartAnchorPoint = FGeometryHelper::FindClosestPointOnGeom(StartGeom, SeedPoint);
	const FVector2D EndAnchorPoint = FGeometryHelper::FindClosestPointOnGeom(EndGeom, SeedPoint);

	DrawSplineWithArrow(StartAnchorPoint, EndAnchorPoint, Params);
}

FVector2D FMontageGraphConnectionDrawingPolicy::ComputeSplineTangent(const FVector2D& Start,
                                                                     const FVector2D& End) const
{
	const FVector2D Delta = End - Start;
	const FVector2D NormDelta = Delta.GetSafeNormal();

	return NormDelta;
}

void FMontageGraphConnectionDrawingPolicy::DetermineLinkGeometry(FArrangedChildren& ArrangedNodes,
                                                                 TSharedRef<SWidget>& OutputPinWidget,
                                                                 UEdGraphPin* OutputPin, UEdGraphPin* InputPin,
                                                                 FArrangedWidget*& StartWidgetGeometry,
                                                                 FArrangedWidget*& EndWidgetGeometry)
{
	//
	// if (UMontageGraphEdNodeEntry* EntryNode = Cast<UMontageGraphEdNodeEntry>(OutputPin->GetOwningNode()))
	// {
	// 	StartWidgetGeometry = PinGeometries->Find(OutputPinWidget);
	//
	// 	const UMontageGraphEdNode* OwningNode = CastChecked<UMontageGraphEdNode>(InputPin->GetOwningNode());
	// 	const int32 Index = NodeWidgetMap.FindChecked(OwningNode);
	// 	EndWidgetGeometry = &(ArrangedNodes[Index]);
	// }
	// else if (UMontageGraphEdNodeMontage* MontageNode = Cast<UMontageGraphEdNodeMontage>(OutputPin->GetOwningNode()))
	// {
	// 	
	// }
	// else if (const UMontageGraphEdNodeEdge* EdgeNode = Cast<UMontageGraphEdNodeEdge>(InputPin->GetOwningNode()))
	// {
	// 	UMontageGraphEdNode* Start = EdgeNode->GetStartNodeAsBase();
	// 	UMontageGraphEdNode* End = EdgeNode->GetEndNode();
	// 	if (Start != nullptr && End != nullptr)
	// 	{
	// 		int32* StartNodeIndex = NodeWidgetMap.Find(Start);
	// 		int32* EndNodeIndex = NodeWidgetMap.Find(End);
	// 		if (StartNodeIndex != nullptr && EndNodeIndex != nullptr)
	// 		{
	// 			StartWidgetGeometry = &(ArrangedNodes[*StartNodeIndex]);
	// 			EndWidgetGeometry = &(ArrangedNodes[*EndNodeIndex]);
	// 		}
	// 	}
	// }
	// else if (UMontageGraphEdNodeSelector* SelectorNode = Cast<UMontageGraphEdNodeSelector>(OutputPin->GetOwningNode()))
	// {
	// 	StartWidgetGeometry = PinGeometries->Find(OutputPinWidget);
	//
	// 	const UMontageGraphEdNode* OwningNode = CastChecked<UMontageGraphEdNode>(InputPin->GetOwningNode());
	// 	const int32                 Index      = NodeWidgetMap.FindChecked(OwningNode);
	// 	EndWidgetGeometry                      = &(ArrangedNodes[Index]);
	// }

	StartWidgetGeometry = PinGeometries->Find(OutputPinWidget);

	const UMontageGraphEdNode* OwningNode = CastChecked<UMontageGraphEdNode>(InputPin->GetOwningNode());
	const int32 Index = NodeWidgetMap.FindChecked(OwningNode);
	EndWidgetGeometry = &(ArrangedNodes[Index]);
}
