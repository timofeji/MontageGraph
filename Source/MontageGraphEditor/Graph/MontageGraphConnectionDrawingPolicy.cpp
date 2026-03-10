#include "MontageGraphConnectionDrawingPolicy.h"
#include "MontageEdGraph.h"
#include "Slate/SMontageGraphSelectorOutputPin.h"
#include "EdNodes/MGEdNode.h"
#include "EdNodes/MGEdNode_Edge.h"

FMontageGraphConnectionDrawingPolicy::FMontageGraphConnectionDrawingPolicy(
	const int32 InBackLayerID, const int32 InFrontLayerID, const float ZoomFactor, const FSlateRect& InClippingRect,
	FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj)
	: FConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, ZoomFactor, InClippingRect, InDrawElements)
	  , GraphObj(InGraphObj)
{
	MGEdGraph = Cast<UMontageEdGraph>(GraphObj);
}

void FMontageGraphConnectionDrawingPolicy::DetermineWiringStyle(UEdGraphPin*       OutputPin, UEdGraphPin* InputPin,
                                                                FConnectionParams& Params)
{
	Params.AssociatedPin1 = OutputPin;
	Params.AssociatedPin2 = InputPin;
	Params.WireThickness  = 1.5f;


	if (MGEdGraph && InputPin)
	{
		FLinearColor BaseWireColor(1.00f, 1.00f, 1.00f, 0.43f);
		Params.WireColor = BaseWireColor;
		if (UMGEdNode_Edge* Edge = Cast<UMGEdNode_Edge>(InputPin->GetOwningNode()))
		{
			Params.WireColor   = Edge->GetEdgeColor();
			Params.WireColor.A = BaseWireColor.A;
		}

		if (UMGEdNode* MGEdNode = Cast<UMGEdNode>(InputPin->GetOwningNode()))
		{
			MGEdNode->UpdateWireConnectionParams(Params);
		}
	}

	if (HoveredPins.Num() > 0)
	{
		ApplyHoverDeemphasis(OutputPin, InputPin, Params.WireThickness, Params.WireColor);
	}
}


void FMontageGraphConnectionDrawingPolicy::BuildPinToPinWidgetMap(
	TMap<TSharedRef<SWidget>, FArrangedWidget>& InPinGeometries)
{
	FConnectionDrawingPolicy::BuildPinToPinWidgetMap(InPinGeometries);

	// // Add any sub-pins to the widget map if they arent there already, but with their parents geometry.
	for (TMap<TSharedRef<SWidget>, FArrangedWidget>::TIterator ConnectorIt(InPinGeometries); ConnectorIt; ++ConnectorIt)
	{
		struct Local
		{
			static void AddSubPins_Recursive(UEdGraphPin* PinObj,
											 TMap<UEdGraphPin*, TSharedPtr<SGraphPin>>& InPinToPinWidgetMap,
											 TSharedPtr<SGraphPin>& InGraphPinWidget)
			{
				for (UEdGraphPin* SubPin : PinObj->SubPins)
				{
					// Only add to the pin-to-pin widget map if the sub-pin widget is not there already
					TSharedPtr<SGraphPin>* SubPinWidgetPtr = InPinToPinWidgetMap.Find(SubPin);
					if (SubPinWidgetPtr == nullptr)
					{
						SubPinWidgetPtr = &InGraphPinWidget;
					}
	
					TSharedPtr<SGraphPin> PinWidgetPtr = *SubPinWidgetPtr;
					InPinToPinWidgetMap.Add(SubPin, PinWidgetPtr);
					AddSubPins_Recursive(SubPin, InPinToPinWidgetMap, PinWidgetPtr);
				}
			}
		};
	
	
		TSharedPtr<SGraphPin> GraphPinWidget = StaticCastSharedRef<SGraphPin>(ConnectorIt.Key());
		auto PinObj = GraphPinWidget->GetPinObj();
		if (PinObj)
		{
			Local::AddSubPins_Recursive(PinObj, PinToPinWidgetMap, GraphPinWidget);
		}
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

	PinGeometries = &InPinGeometries;

	BuildPinToPinWidgetMap(InPinGeometries);

	DrawPinGeometries(InPinGeometries, ArrangedNodes);
}

void FMontageGraphConnectionDrawingPolicy::DrawPreviewConnector(const FGeometry& PinGeometry,
                                                                const FVector2f& StartPoint, const FVector2f& EndPoint,
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



void FMontageGraphConnectionDrawingPolicy::DrawPinGeometries(TMap<TSharedRef<SWidget>, FArrangedWidget>& InPinGeometries, FArrangedChildren& ArrangedNodes)
{
	for (TMap<TSharedRef<SWidget>, FArrangedWidget>::TIterator ConnectorIt(InPinGeometries); ConnectorIt; ++ConnectorIt)
	{
		TSharedRef<SWidget> SomePinWidget = ConnectorIt.Key();
		SGraphPin& PinWidget = static_cast<SGraphPin&>(SomePinWidget.Get());
		UEdGraphPin* ThePin = PinWidget.GetPinObj();

		if (ThePin && ThePin->Direction == EGPD_Output)
		{
			for (int32 LinkIndex=0; LinkIndex < ThePin->LinkedTo.Num(); ++LinkIndex)
			{
				FArrangedWidget* LinkStartWidgetGeometry = nullptr;
				FArrangedWidget* LinkEndWidgetGeometry = nullptr;

				UEdGraphPin* TargetPin = ThePin->LinkedTo[LinkIndex];

				DetermineLinkGeometry(ArrangedNodes, SomePinWidget, ThePin, TargetPin, /*out*/ LinkStartWidgetGeometry, /*out*/ LinkEndWidgetGeometry);

				if (( LinkEndWidgetGeometry && LinkStartWidgetGeometry ) && !IsConnectionCulled( *LinkStartWidgetGeometry, *LinkEndWidgetGeometry ))
				{
					FConnectionParams Params;
					DetermineWiringStyle(ThePin, TargetPin, /*inout*/ Params);
					const TSharedPtr<SGraphPin>* ConnectedPinWidget = PinToPinWidgetMap.Find(TargetPin);
					if (ConnectedPinWidget && ConnectedPinWidget->IsValid())
					{
						if ( PinWidget.AreConnectionsFaded() && (*ConnectedPinWidget)->AreConnectionsFaded() )
						{
							Params.WireColor.A = 0.2f;
						}
					}
					DrawSplineWithArrow(LinkStartWidgetGeometry->Geometry, LinkEndWidgetGeometry->Geometry, Params);
				}
			}
		}
	}
}

void FMontageGraphConnectionDrawingPolicy::DrawSplineWithArrow(const FVector2f& StartPoint, const FVector2f& EndPoint,
                                                               const FConnectionParams& Params)
{
	// bUserFlag1 indicates that we need to reverse the direction of connection (used by debugger)
	const FVector2f& P0 = Params.bUserFlag1 ? EndPoint : StartPoint;
	const FVector2f& P1 = Params.bUserFlag1 ? StartPoint : EndPoint;

	Internal_DrawLineWithArrow(P0, P1, Params);
}

void FMontageGraphConnectionDrawingPolicy::Internal_DrawLineWithArrow(const FVector2f&         StartPoint,
                                                                      const FVector2f&         EndPoint,
                                                                      const FConnectionParams& Params)
{
	const FVector2f Dir = (EndPoint - StartPoint).GetSafeNormal();
								// + FVector2f(0.f, 0.1f); //Bias Horizontal Direction
	const FVector2f CardinalDir = FMath::Abs(Dir.X) > FMath::Abs(Dir.Y)
		                              ? FVector2f(FMath::Sign(Dir.X), 0.f)
		                              : FVector2f(0.f, FMath::Sign(Dir.Y));

	const FVector2f Mid = CardinalDir * LineSeparationAmount * ZoomFactor;
	const FVector2f P2  = StartPoint + Mid;
	const FVector2f P3  = EndPoint - Mid;

	// Draw a line/spline
	DrawConnection(WireLayerID, StartPoint, P2, Params);
	DrawConnection(WireLayerID, P2, P3, Params);
	DrawConnection(WireLayerID, P3, EndPoint, Params);


	// Draw pin type
	const FSlateBrush* BrushToDraw = ArrowImage;
	const FVector2f EndImgDrawPos = EndPoint - .5f * BrushToDraw->ImageSize * ZoomFactor;
	const float AngleInRadians = static_cast<float>(FMath::Atan2(CardinalDir.Y, CardinalDir.X));
	
	if (Params.AssociatedPin2)
	{
		FSlateDrawElement::MakeRotatedBox(
			DrawElementsList,
			ArrowLayerID + 5,
			FPaintGeometry(EndImgDrawPos, BrushToDraw->ImageSize * ZoomFactor, ZoomFactor),
			BrushToDraw,
			ESlateDrawEffect::None,
			AngleInRadians,
			TOptional<FVector2f>(),
			FSlateDrawElement::RelativeToElement,
			Params.WireColor
		);
	}
}

FVector2f FMontageGraphConnectionDrawingPolicy::FindClosestEdgeMidpointOnGeometry(const FGeometry& StartGeom, const FVector2f SeedPoint)
{
	TArray<FVector2f> Points;
	FGeometryHelper::ConvertToPoints(StartGeom, Points);

	float     BestDistanceSquared = MAX_FLT;
	FVector2f BestPoint                     = FVector2f::ZeroVector;
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		const FVector2f Candidate                = .5f * (Points[i] + Points[(i + 1) % Points.Num()]);
		const float     CandidateDistanceSquared = (Candidate - SeedPoint).SizeSquared();
		if (CandidateDistanceSquared < BestDistanceSquared)
		{
			BestPoint           = Candidate;
			BestDistanceSquared = CandidateDistanceSquared;
		}
	}
	return BestPoint;
}

void FMontageGraphConnectionDrawingPolicy::DrawSplineWithArrow(const FGeometry& StartGeom, const FGeometry& EndGeom,
                                                               const FConnectionParams& Params)
{
	// Get a reasonable seed point (halfway between the boxes)
	const FVector2f StartCenter = FGeometryHelper::CenterOf(StartGeom);
	const FVector2f EndCenter = FGeometryHelper::CenterOf(EndGeom);
	const FVector2f SeedPoint = (StartCenter + EndCenter) * 0.5;

	//*Aggregate Spline start/end positions to geometry edge midpoints*//
	const FVector2f StartAnchorPoint = FindClosestEdgeMidpointOnGeometry(StartGeom, SeedPoint);;
	const FVector2f EndAnchorPoint   = FindClosestEdgeMidpointOnGeometry(EndGeom, SeedPoint);;

	DrawSplineWithArrow(StartAnchorPoint, EndAnchorPoint, Params);
}

FVector2f FMontageGraphConnectionDrawingPolicy::ComputeSplineTangent(const FVector2f& Start,
                                                                     const FVector2f& End) const
{
	const FVector2f Delta = End - Start;
	const FVector2f NormDelta = Delta.GetSafeNormal();

	return NormDelta;
}
//
void FMontageGraphConnectionDrawingPolicy::DetermineLinkGeometry(FArrangedChildren&   ArrangedNodes,
                                                                 TSharedRef<SWidget>& OutputPinWidget,
                                                                 UEdGraphPin*         OutputPin, UEdGraphPin* InputPin,
                                                                 FArrangedWidget*&    StartWidgetGeometry,
                                                                 FArrangedWidget*&    EndWidgetGeometry)
{
	if (const TSharedPtr<SGraphPin>* OutputWidgetPtr = PinToPinWidgetMap.Find(OutputPin))
	{
		StartWidgetGeometry = PinGeometries->Find((*OutputWidgetPtr).ToSharedRef());
	}

	//Since our "input" pins are hidden, we wire to the output pin geometry instead
	if (const TSharedPtr<SGraphPin>* InputWidgetPtr = PinToPinWidgetMap.Find(InputPin->GetOwningNode()->Pins[1]))
	{
		EndWidgetGeometry = PinGeometries->Find((*InputWidgetPtr).ToSharedRef());
	}
	

	if (UMGEdNode_Edge* EdgeNode = Cast<UMGEdNode_Edge>(InputPin->GetOwningNode()))
	{
		UMGEdNode* PrevNode = EdgeNode->GetStartNode();
		UMGEdNode* NextNode = EdgeNode->GetEndNode();
		if ((PrevNode != nullptr) && (NextNode != nullptr))
		{
			int32* PrevNodeIndex = NodeWidgetMap.Find(PrevNode);
			int32* NextNodeIndex = NodeWidgetMap.Find(NextNode);
			if ((PrevNodeIndex != nullptr) && (NextNodeIndex != nullptr))
			{
				StartWidgetGeometry = &(ArrangedNodes[*PrevNodeIndex]);
				EndWidgetGeometry   = &(ArrangedNodes[*NextNodeIndex]);
			}
		}
	}
	// else
	// {
	// 	StartWidgetGeometry = PinGeometries->Find(OutputPinWidget);
	//
	// 	if (TSharedPtr<SGraphPin>* pTargetWidget = PinToPinWidgetMap.Find(InputPin))
	// 	{
	// 		TSharedRef<SGraphPin> InputWidget = (*pTargetWidget).ToSharedRef();
	// 		EndWidgetGeometry = PinGeometries->Find(InputWidget);
	// 	}
	// }
}
