#include "Level_CombinedSteering.h"
#include "imgui.h"


ALevel_CombinedSteering::ALevel_CombinedSteering()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ALevel_CombinedSteering::BeginPlay()
{
	Super::BeginPlay();

	// drunkagent setup
	DrunkAgent = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass, FVector{ -300.f, 0.f, 90.f }, FRotator::ZeroRotator);
	if (DrunkAgent)
	{
		DrunkSeekBehavior = std::make_unique<Seek>();
		DrunkWanderBehavior = std::make_unique<Wander>();

		std::vector<BlendedSteering::WeightedBehavior> DrunkWeights;
		DrunkWeights.push_back({ DrunkSeekBehavior.get(), 0.5f });
		DrunkWeights.push_back({ DrunkWanderBehavior.get(), 0.5f });

		DrunkBlendedBehavior = std::make_unique<BlendedSteering>(DrunkWeights);
		DrunkAgent->SetSteeringBehavior(DrunkBlendedBehavior.get());
	}

	// evadingagent setupm
	EvadingAgent = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass, FVector{ 300.f, 0.f, 90.f }, FRotator::ZeroRotator);
	if (EvadingAgent)
	{
		EvadingEvadeBehavior = std::make_unique<Evade>();
		EvadingWanderBehavior = std::make_unique<Wander>();

		std::vector<ISteeringBehavior*> EvadingPriorities;
		EvadingPriorities.push_back(EvadingEvadeBehavior.get());
		EvadingPriorities.push_back(EvadingWanderBehavior.get());

		EvadingPriorityBehavior = std::make_unique<PrioritySteering>(EvadingPriorities);
		EvadingAgent->SetSteeringBehavior(EvadingPriorityBehavior.get());
	}
}

void ALevel_CombinedSteering::BeginDestroy()
{
	Super::BeginDestroy();
}

void ALevel_CombinedSteering::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (DrunkAgent && DrunkSeekBehavior)
	{
		DrunkSeekBehavior->SetTarget(MouseTarget);
	}

	if (EvadingAgent && EvadingEvadeBehavior && DrunkAgent)
	{
		FTargetData DrunkTargetData;
		DrunkTargetData.Position = DrunkAgent->GetPosition();
		DrunkTargetData.LinearVelocity = DrunkAgent->GetLinearVelocity();
		DrunkTargetData.Orientation = DrunkAgent->GetRotation();
		DrunkTargetData.AngularVelocity = DrunkAgent->GetAngularVelocity();

		EvadingEvadeBehavior->SetTarget(DrunkTargetData);
	}

#pragma region UI
	//UI
	{
		//Setup
		bool windowActive = true;
		ImGui::SetNextWindowPos(WindowPos);
		ImGui::SetNextWindowSize(WindowSize);
		ImGui::Begin("Game AI", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Text("LMB: place target");
		ImGui::Text("RMB: move cam.");
		ImGui::Text("Scrollwheel: zoom cam.");
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("Combined Steering");
		ImGui::Spacing();
		ImGui::Spacing();

		if (ImGui::Checkbox("Debug Rendering", &CanDebugRender))
		{
			// debug rendering
			if (DrunkAgent) DrunkAgent->SetDebugRenderingEnabled(CanDebugRender);
			if (EvadingAgent) EvadingAgent->SetDebugRenderingEnabled(CanDebugRender);
		}

		ImGui::Checkbox("Trim World", &TrimWorld->bShouldTrimWorld);
		if (TrimWorld->bShouldTrimWorld)
		{
			ImGuiHelpers::ImGuiSliderFloatWithSetter("Trim Size",
				TrimWorld->GetTrimWorldSize(), 1000.f, 3000.f,
				[this](float InVal) { TrimWorld->SetTrimWorldSize(InVal); });
		}

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("Behavior Weights (Drunk Agent)");
		ImGui::Spacing();

		if (DrunkBlendedBehavior && DrunkBlendedBehavior->GetWeightedBehaviorsRef().size() >= 2)
		{
			ImGuiHelpers::ImGuiSliderFloatWithSetter("Seek",
				DrunkBlendedBehavior->GetWeightedBehaviorsRef()[0].Weight, 0.f, 1.f,
				[this](float InVal) { DrunkBlendedBehavior->GetWeightedBehaviorsRef()[0].Weight = InVal; }, "%.2f");

			ImGuiHelpers::ImGuiSliderFloatWithSetter("Wander",
				DrunkBlendedBehavior->GetWeightedBehaviorsRef()[1].Weight, 0.f, 1.f,
				[this](float InVal) { DrunkBlendedBehavior->GetWeightedBehaviorsRef()[1].Weight = InVal; }, "%.2f");
		}

		//End
		ImGui::End();
	}
#pragma endregion

}