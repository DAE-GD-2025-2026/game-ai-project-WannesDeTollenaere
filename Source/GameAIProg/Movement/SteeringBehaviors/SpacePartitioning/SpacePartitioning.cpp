#include "SpacePartitioning.h"

// --- Cell ---
// ------------
Cell::Cell(float Left, float Bottom, float Width, float Height)
{
	BoundingBox.Min = { Left, Bottom };
	BoundingBox.Max = { BoundingBox.Min.X + Width, BoundingBox.Min.Y + Height };
}

std::vector<FVector2D> Cell::GetRectPoints() const
{
	const float left = BoundingBox.Min.X;
	const float bottom = BoundingBox.Min.Y;
	const float width = BoundingBox.Max.X - BoundingBox.Min.X;
	const float height = BoundingBox.Max.Y - BoundingBox.Min.Y;

	std::vector<FVector2D> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(UWorld* pWorld, float Width, float Height, int Rows, int Cols, int MaxEntities)
	: pWorld{pWorld}
	, SpaceWidth{Width}
	, SpaceHeight{Height}
	, NrOfRows{Rows}
	, NrOfCols{Cols}
	, NrOfNeighbors{0}
{
	Neighbors.SetNum(MaxEntities);
	
	//calculate bounds of a cell
	CellWidth = Width / Cols;
	CellHeight = Height / Rows;


	CellOrigin = FVector2D(-Width / 2.0f, -Height / 2.0f);

	// create the cells
	Cells.reserve(NrOfRows * NrOfCols);
	for (int r = 0; r < NrOfRows; ++r)
	{
		for (int c = 0; c < NrOfCols; ++c)
		{
			float left = CellOrigin.X + (c * CellWidth);
			float bottom = CellOrigin.Y + (r * CellHeight);
			Cells.emplace_back(left, bottom, CellWidth, CellHeight);
		}
	}
}

void CellSpace::AddAgent(ASteeringAgent& Agent)
{
	int index = PositionToIndex(Agent.GetPosition());
	Cells[index].Agents.push_back(&Agent);
}

void CellSpace::UpdateAgentCell(ASteeringAgent& Agent, const FVector2D& OldPos)
{
	int oldIndex = PositionToIndex(OldPos);
	int newIndex = PositionToIndex(Agent.GetPosition());

	if (oldIndex != newIndex)
	{
		Cells[oldIndex].Agents.remove(&Agent);
		Cells[newIndex].Agents.push_back(&Agent);
	}
}

void CellSpace::RegisterNeighbors(ASteeringAgent& Agent, float QueryRadius)
{
	// Register the neighbors for the provided agent
	// Only check the cells that are within the radius of the neighborhood
	NrOfNeighbors = 0;
	//instead of checking overlap for every cell for every agent i just check wich cells needs to be checked via math

	FVector2D agentPos = Agent.GetPosition();
	FRect queryBox;
	queryBox.Min = agentPos - FVector2D(QueryRadius, QueryRadius);
	queryBox.Max = agentPos + FVector2D(QueryRadius, QueryRadius);

	float queryRadiusSq = QueryRadius * QueryRadius;

	int minCol = FMath::FloorToInt((queryBox.Min.X - CellOrigin.X) / CellWidth);
	int minRow = FMath::FloorToInt((queryBox.Min.Y - CellOrigin.Y) / CellHeight);
	int maxCol = FMath::FloorToInt((queryBox.Max.X - CellOrigin.X) / CellWidth);
	int maxRow = FMath::FloorToInt((queryBox.Max.Y - CellOrigin.Y) / CellHeight);

	// clamop in bounds so no out of range exceptions
	minCol = FMath::Clamp(minCol, 0, NrOfCols - 1);
	minRow = FMath::Clamp(minRow, 0, NrOfRows - 1);
	maxCol = FMath::Clamp(maxCol, 0, NrOfCols - 1);
	maxRow = FMath::Clamp(maxRow, 0, NrOfRows - 1);

	for (int row = minRow; row <= maxRow; ++row)
	{
		for (int col = minCol; col <= maxCol; ++col)
		{
			int index = (row * NrOfCols) + col;
			const Cell& c = Cells[index];

			for (ASteeringAgent* pOtherAgent : c.Agents)
			{
				if (pOtherAgent != nullptr && pOtherAgent != &Agent)
				{
					if (FVector2D::DistSquared(agentPos, pOtherAgent->GetPosition()) <= queryRadiusSq)
					{
						if (NrOfNeighbors < Neighbors.Num())
						{
							Neighbors[NrOfNeighbors++] = pOtherAgent;
						}
					}
				}
			}
		}
	}
}

void CellSpace::EmptyCells()
{
	for (Cell& c : Cells)
		c.Agents.clear();
}

void CellSpace::RenderCells() const
{
	//  Render the cells with the number of agents inside of it

	if (!pWorld) return;

	for (const Cell& c : Cells)
	{
		// boxes
		FVector center(c.BoundingBox.Min.X + CellWidth / 2.0f, c.BoundingBox.Min.Y + CellHeight / 2.0f, 0.0f);
		FVector extents(CellWidth / 2.0f, CellHeight / 2.0f, 0.0f);
		DrawDebugBox(pWorld, center, extents, FColor::Blue, false, -1.0f, 0, 2.0f);

		// amount of agents
		FString text = FString::Printf(TEXT("%d"), c.Agents.size());
		DrawDebugString(pWorld, center, text, nullptr, FColor::White, 0.0f, true);
	}
}

int CellSpace::PositionToIndex(FVector2D const & Pos) const
{
	//  Calculate the index of the cell based on the position
	int col = FMath::FloorToInt((Pos.X - CellOrigin.X) / CellWidth);
	int row = FMath::FloorToInt((Pos.Y - CellOrigin.Y) / CellHeight);

	col = FMath::Clamp(col, 0, NrOfCols - 1);
	row = FMath::Clamp(row, 0, NrOfRows - 1);

	return (row * NrOfCols) + col;
}

bool CellSpace::DoRectsOverlap(FRect const & RectA, FRect const & RectB)
{
	// Check if the rectangles are separated on either axis
	if (RectA.Max.X < RectB.Min.X || RectA.Min.X > RectB.Max.X) return false;
	if (RectA.Max.Y < RectB.Min.Y || RectA.Min.Y > RectB.Max.Y) return false;
    
	// If they are not separated, they must overlap
	return true;
}