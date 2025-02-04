#include "pch.hpp"
#include "xmlMapParser.hpp"
#include "Components/physicsComponents.hpp"
#include "Components/debugComponents.hpp"
#include "Components/objectsComponents.hpp"
#include "Renderer/renderer.hpp"
#include "AI/aiManager.hpp"
#include "Utilities/csv.hpp"
#include "Utilities/filePath.hpp"
#include "Utilities/random.hpp"

namespace ph {

void XmlMapParser::parseFile(const Xml& mapNode, AIManager& aiManager, entt::registry& gameRegistry, EntitiesTemplateStorage& templates)
{
	mGameRegistry = &gameRegistry;
	mTemplates = &templates;

	//load denial areas
	auto objectGroups = mapNode.getChildren("objectgroup");
	for(auto& objectGroup : objectGroups)
	{
		if(objectGroup.getAttribute("name")->toString() == "denialAreas")
		{
			auto objects = objectGroup.getChildren("object");
			for(auto& object : objects)
			{
				auto getBounds = [&object]
				{
					return FloatRect(object.getAttribute("x")->toFloat(), object.getAttribute("y")->toFloat(),
					                 object.getAttribute("width")->toFloat(), object.getAttribute("height")->toFloat());
				};

				if(object.getAttribute("type")->toString() == "CollisionAndLightWallDenialArea")
					mDenialAreas.collisionsAndLightWalls.emplace_back(getBounds());
				else if(object.getAttribute("type")->toString() == "CollisionDenialArea")
					mDenialAreas.collisions.emplace_back(getBounds());
				else if(object.getAttribute("type")->toString() == "LightWallDenialArea")
					mDenialAreas.lightWalls.emplace_back(getBounds());
			}
		}
	}

	#ifndef PH_DISTRIBUTION
	{
		// load denial areas to registry for debug visualization purposes

		using component::DenialArea;

		auto createDenialArea = [this](FloatRect area, DenialArea::Type type)
		{
			auto entity = mGameRegistry->create();
			mGameRegistry->assign<component::BodyRect>(entity, area);
			mGameRegistry->assign<component::DenialArea>(entity, DenialArea{type});
		};

		for(auto& area : mDenialAreas.collisions)
			createDenialArea(area, DenialArea::Collision);

		for(auto& area : mDenialAreas.lightWalls)
			createDenialArea(area, DenialArea::LightWall);

		for(auto& area : mDenialAreas.collisionsAndLightWalls)
			createDenialArea(area, DenialArea::All);
	}
	#endif

	GeneralMapInfo info = getGeneralMapInfo(mapNode);

	aiManager.registerMapSize(Cast<Vec2u>(info.mapSize));
	aiManager.registerTileSize(info.tileSize);

	mRenderChunks.reserve(Cast<size_t>(info.nrOfChunks));
	mChunkCollisions.reserve(Cast<size_t>(info.nrOfChunks));

	const std::vector<Xml> tilesetNodes = mapNode.getChildren("tileset");
	PH_ASSERT_WARNING(tilesetNodes.size() != 0, "Map doesn't have any tilesets");

	const TilesetsData tilesetsData = getTilesetsData(tilesetNodes);
	const std::vector<Xml> layerNodes = getLayerNodes(mapNode);
	
	// parse map layers
	FloatRect mapBounds;
	u8 z = sLowestLayerZ;
	bool isFirstChunk = true;
	for (const Xml& layerNode : layerNodes)
	{
		Xml dataNode = *layerNode.getChild("data");
		std::string encoding = dataNode.getAttribute("encoding")->toString();
		PH_ASSERT_CRITICAL(encoding == "csv", "Used unsupported data encoding: " + encoding);
		auto layerName = layerNode.getAttribute("name")->toString();
		bool outdoor = layerName.find("indoor") == std::string::npos;
		for(Xml& chunkNode : dataNode.getChildren("chunk"))
		{
			Vec2 chunkPos(chunkNode.getAttribute("x")->toFloat(), chunkNode.getAttribute("y")->toFloat());
			Vec2 chunkSize(chunkNode.getAttribute("width")->toFloat(), chunkNode.getAttribute("height")->toFloat());

			PH_ASSERT_CRITICAL(chunkSize.x == 12.f, "You have to set map parameter \"Output Chunk Width\" to 12!");
			PH_ASSERT_CRITICAL(chunkSize.y == 12.f, "You have to set map parameter \"Output Chunk Height\" to 12!");

			if(isFirstChunk) 
			{
				mapBounds.pos = chunkPos;
				mapBounds.size = chunkSize;
				isFirstChunk = false;
			}
			else
			{
				if(chunkPos.x < mapBounds.x) {
					mapBounds.x = chunkPos.x;
				}
				if(chunkPos.y < mapBounds.y) {
					mapBounds.y = chunkPos.y;
				}
				
				float mapWidthToThisChunk = chunkPos.x - mapBounds.x + chunkSize.x;
				if(mapWidthToThisChunk > mapBounds.w) {
					mapBounds.w = mapWidthToThisChunk; 
				}
				float mapHeightToThisChunk = chunkPos.y - mapBounds.y + chunkSize.y;
				if(mapHeightToThisChunk > mapBounds.h) {
					mapBounds.h = mapHeightToThisChunk;
				}
			}

			auto globalIds = Csv::toU32s(chunkNode.toString());
			createChunk(chunkPos, globalIds, tilesetsData, info, z, aiManager, outdoor);
		}
		z -= 2;
	}

	// translate map bounds to world space
	mapBounds.x *= info.tileSize.x;
	mapBounds.y *= info.tileSize.y;
	mapBounds.w *= info.tileSize.x;
	mapBounds.h *= info.tileSize.y;
	
	auto createBorderCollision = [this]() -> FloatRect& {
		auto borderEntity = mTemplates->createCopy("BorderCollision", *mGameRegistry);
		return mGameRegistry->get<component::BodyRect>(borderEntity);
	};

	// create left map border
	auto& leftBorderRect = createBorderCollision();
	leftBorderRect = FloatRect(mapBounds.x - info.tileSize.x, mapBounds.y - info.tileSize.y, info.tileSize.x, mapBounds.h + 2 * info.tileSize.y);

	// create top map border
	auto& topBorderRect = createBorderCollision();
	topBorderRect = FloatRect(mapBounds.x - info.tileSize.x, mapBounds.y - info.tileSize.y, mapBounds.w + 2 * info.tileSize.x, info.tileSize.y);

	// create right map border
	auto& rightBorderRect = createBorderCollision();
	rightBorderRect = FloatRect(mapBounds.w + mapBounds.x, -info.tileSize.y + mapBounds.y, info.tileSize.x, mapBounds.h + 2 * info.tileSize.y);

	// create bottom map border
	auto& bottomBorderRect = createBorderCollision();
	bottomBorderRect = FloatRect(mapBounds.x - info.tileSize.x, mapBounds.h + mapBounds.y, mapBounds.w + 2 * info.tileSize.x, info.tileSize.y);
}

auto XmlMapParser::getGeneralMapInfo(const Xml& mapNode) const -> GeneralMapInfo
{
	const std::string orientation = mapNode.getAttribute("orientation")->toString();
	PH_ASSERT_CRITICAL(orientation == "orthogonal", "Used unsupported map orientation: " + orientation);
	PH_ASSERT_CRITICAL(mapNode.getAttribute("infinite")->toBool(), "Now we support only infinite maps!");

	GeneralMapInfo info;
	info.mapSize.x = mapNode.getAttribute("width")->toFloat();
	info.mapSize.y = mapNode.getAttribute("height")->toFloat();
	info.tileSize.x = mapNode.getAttribute("tilewidth")->toFloat();
	info.tileSize.y = mapNode.getAttribute("tileheight")->toFloat();
	info.nrOfChunksInOneRow = std::ceil(info.mapSize.x / sChunkSize);
	info.nrOfChunksInOneColumn = std::ceil(info.mapSize.y / sChunkSize);
	info.nrOfChunks = info.nrOfChunksInOneRow * info.nrOfChunksInOneColumn;
	return info;
}

auto XmlMapParser::getTilesetsData(const std::vector<Xml>& tilesetNodes) const -> const TilesetsData
{
	TilesetsData tilesets;
	tilesets.firstGlobalTileIds.reserve(tilesetNodes.size());
	tilesets.tileCounts.reserve(tilesetNodes.size());
	tilesets.columnsCounts.reserve(tilesetNodes.size());

	for(Xml tilesetNode : tilesetNodes) 
	{
		const u32 firstGlobalTileId = tilesetNode.getAttribute("firstgid")->toU32();
		tilesets.firstGlobalTileIds.emplace_back(firstGlobalTileId);
		if(auto source = tilesetNode.getAttribute("source")) {
			std::string tilesetNodeSource = source->toString();
			tilesetNodeSource = FilePath::toFilename(tilesetNodeSource, '/');
			PH_LOG_INFO("Detected not embedded tileset in Map: " + tilesetNodeSource);
			Xml tilesetDocument;
			PH_ASSERT_CRITICAL(tilesetDocument.loadFromFile("scenes/map/" + tilesetNodeSource),
				"Not embedded tileset file \"" + tilesetNodeSource + "\" wasn't loaded correctly!");
			tilesetNode = *tilesetDocument.getChild("tileset");
		}
		tilesets.tileCounts.emplace_back(tilesetNode.getAttribute("tilecount")->toU32());
		tilesets.columnsCounts.emplace_back(tilesetNode.getAttribute("columns")->toU32());
		const Xml imageNode = *tilesetNode.getChild("image");
		tilesets.tilesetFileName = FilePath::toFilename(imageNode.getAttribute("source")->toString(), '/');
		const std::vector<Xml> tileNodes = tilesetNode.getChildren("tile");
		TilesData tilesData = getTilesData(tileNodes);
		tilesData.firstGlobalTileId = firstGlobalTileId;
		tilesets.tilesData.emplace_back(tilesData);
	}

	return tilesets;
}

auto XmlMapParser::getTilesData(const std::vector<Xml>& tileNodes) const -> TilesData
{
	TilesData tilesData{};
	tilesData.ids.reserve(tileNodes.size());
	tilesData.rectCollisions.reserve(tileNodes.size());
	for(const Xml& tileNode : tileNodes) 
	{
		if(auto objectGroupNode = tileNode.getChild("objectgroup"))
		{
			tilesData.ids.emplace_back(tileNode.getAttribute("id")->toU32());
			const auto objectNodes = objectGroupNode->getChildren("object");
			std::vector<FloatRect> rectCollisions;
			std::vector<component::BodyCircle> circleCollisions;
			std::vector<FloatRect> lightWalls;
			bool puzzleGridRoad = false;
			Pit pit;
			for(auto& objectNode : objectNodes)
			{
				if(auto type = objectNode.getAttribute("type"))
				{
					auto getBounds = [&objectNode] 
					{
						auto width = objectNode.getAttribute("width");
						auto height = objectNode.getAttribute("height");
						return FloatRect(
							objectNode.getAttribute("x")->toFloat(),
							objectNode.getAttribute("y")->toFloat(),
							width ? width->toFloat() : 0.f,
							height ? height->toFloat() : 0.f
						);
					};

					std::string typeStr = type->toString();
					if(typeStr == "Collision")
					{
						if(objectNode.getChild("ellipse"))
						{
							auto bounds = getBounds();
							float radius = bounds.w / 2.f;
							auto circleCenter = bounds.pos + Vec2(radius, radius);
							circleCollisions.emplace_back(component::BodyCircle{circleCenter, radius});
						}
						else
						{
							rectCollisions.emplace_back(getBounds());
						}
					}
					else if(typeStr == "LightWall")
					{
						lightWalls.emplace_back(getBounds());
					}
					else if(typeStr == "PuzzleGridRoad")
					{
						puzzleGridRoad = true;
					}
					else if(typeStr == "Pit")
					{
						PH_ASSERT_UNEXPECTED_SITUATION(!pit.exists, "There can't be more then 1 pit in the same tile");
						pit.bounds = getBounds();
						pit.exists = true;
					}
				}
			}
			tilesData.rectCollisions.emplace_back(rectCollisions);
			tilesData.circleCollisions.emplace_back(circleCollisions);
			tilesData.lightWalls.emplace_back(lightWalls);
			tilesData.puzzleGridRoads.emplace_back(puzzleGridRoad);
			tilesData.pits.emplace_back(pit);
		}
	}
	return tilesData;
}

std::vector<Xml> XmlMapParser::getLayerNodes(const Xml& mapNode) const
{
	const std::vector<Xml> layerNodes = mapNode.getChildren("layer");
	if(layerNodes.size() == 0)
		PH_LOG_WARNING("Map doesn't have any layers");
	return layerNodes;
}

void XmlMapParser::createChunk(Vec2 chunkPos, const std::vector<u32>& globalTileIds, const TilesetsData& tilesets,
                               const GeneralMapInfo& info, u8 z, AIManager& aiManager, bool outdoor)
{
	PH_PROFILE_FUNCTION();

	std::vector<ChunkQuadData> quads;
	std::vector<FloatRect> lightWalls;
	std::vector<FloatRect> chunkCollisionRects;
	std::vector<component::BodyCircle> chunkCollisionCircles;
	FloatRect quadsBounds = FloatRect(chunkPos.x, chunkPos.y, sChunkSize, sChunkSize);
	FloatRect lightWallsBounds = {};

	component::PuzzleGridRoadChunk puzzleGridRoadChunk;
	bool isThereAnyPuzzleGridRoadInThisChunk = false;

	component::PitChunk pitChunk;

	for (size_t tileIndexInChunk = 0; tileIndexInChunk < globalTileIds.size(); ++tileIndexInChunk) 
	{
		constexpr u32 bitsInByte = 8;
		const u32 flippedHorizontally = 1u << (sizeof(u32) * bitsInByte - 1);
		const u32 flippedVertically = 1u << (sizeof(u32) * bitsInByte - 2);
		const u32 flippedDiagonally = 1u << (sizeof(u32) * bitsInByte - 3);

		const bool isHorizontallyFlipped = globalTileIds[tileIndexInChunk] & flippedHorizontally;
		const bool isVerticallyFlipped = globalTileIds[tileIndexInChunk] & flippedVertically;
		const bool isDiagonallyFlipped = globalTileIds[tileIndexInChunk] & flippedDiagonally;

		const u32 globalTileId = globalTileIds[tileIndexInChunk] & (~(flippedHorizontally | flippedVertically | flippedDiagonally));

		bool hasTile = globalTileId != 0;
		if (hasTile) 
		{
			size_t tilesetIndex = findTilesetIndex(globalTileId, tilesets);
			if (tilesetIndex == std::string::npos) 
			{
				PH_LOG_WARNING("It was not possible to find tileset for " + std::to_string(globalTileId));
				continue;
			}

			Vec2u chunkRelativePosInTiles = getTwoDimensionalPositionFromOneDimensionalArrayIndex(Cast<u32>(tileIndexInChunk), Cast<u32>(sChunkSize));
			Vec2 positionInTiles = chunkPos + Cast<Vec2>(chunkRelativePosInTiles);

			// create quad data
			ChunkQuadData cqd;

			Vec2 tileWorldPos( 
				positionInTiles.x * Cast<float>(info.tileSize.x),
				positionInTiles.y * Cast<float>(info.tileSize.y)
			);

			cqd.position = tileWorldPos; 

			// TODO: Replace rotate/size-textureRect stuff with texture coords
			auto tileSize = Cast<Vec2>(info.tileSize);
			if(!(isHorizontallyFlipped || isVerticallyFlipped || isDiagonallyFlipped)) 
			{
				cqd.size = tileSize;
				cqd.rotation = 0.f;
			}
			else if(isHorizontallyFlipped && isVerticallyFlipped && isDiagonallyFlipped) 
			{
				cqd.size = {tileSize.x, -tileSize.y};
				cqd.position.x += tileSize.x;
				cqd.rotation = 270.f;
			}
			else if(isHorizontallyFlipped && isVerticallyFlipped) 
			{
				cqd.size = -tileSize;
				cqd.position += tileSize;
				cqd.rotation = 0.f;
			}
			else if(isHorizontallyFlipped && isDiagonallyFlipped) 
			{
				cqd.size = tileSize;
				cqd.rotation = 90.f;
			}
			else if(isVerticallyFlipped && isDiagonallyFlipped) 
			{
				cqd.size = tileSize;
				cqd.rotation = 270.f;
			}
			else if(isHorizontallyFlipped) 
			{
				cqd.size = {-tileSize.x, tileSize.y};
				cqd.position.x += tileSize.x;
				cqd.rotation = 0.f;
			}
			else if(isVerticallyFlipped) 
			{
				cqd.size = {tileSize.x, -tileSize.y};
				cqd.position.y += tileSize.y;
				cqd.rotation = 0.f;
			}
			else if(isDiagonallyFlipped) 
			{
				cqd.size = {-tileSize.x, tileSize.y};
				cqd.position.y -= tileSize.x;
				cqd.rotation = 270.f;
			}
			cqd.rotation = degreesToRadians(cqd.rotation);

			const u32 tileId = globalTileId - tilesets.firstGlobalTileIds[tilesetIndex];
			auto tileRectPosition = Cast<Vec2>(
				getTwoDimensionalPositionFromOneDimensionalArrayIndex(tileId, tilesets.columnsCounts[tilesetIndex]));
			tileRectPosition.x *= (info.tileSize.x + 2);
			tileRectPosition.y *= (info.tileSize.y + 2);
			tileRectPosition.x += 1;
			tileRectPosition.y += 1;
			const Vec2 textureSize(576.f, 576.f); // TODO: Make it not hardcoded like that
			cqd.textureRect.x = tileRectPosition.x / textureSize.x;
			cqd.textureRect.y = (textureSize.y - tileRectPosition.y - info.tileSize.y) / textureSize.y;
			cqd.textureRect.w = Cast<float>(info.tileSize.x) / textureSize.x;
			cqd.textureRect.h = Cast<float>(info.tileSize.y) / textureSize.y;

			// emplace quad data to chunk
			quads.emplace_back(cqd);

			// load collision bodies and light walls
			size_t tilesDataIndex = findTilesIndex(tilesets.firstGlobalTileIds[tilesetIndex], tilesets.tilesData);
			if(tilesDataIndex == std::string::npos)
				continue;
			auto& tilesData = tilesets.tilesData[tilesDataIndex];
			for(std::size_t i = 0; i < tilesData.ids.size(); ++i) 
			{
				if(tileId == tilesData.ids[i]) 
				{
					// rect collision bodies
					for(FloatRect collisionRect : tilesData.rectCollisions[i])
					{
						if(isHorizontallyFlipped)
							collisionRect.x = info.tileSize.x - collisionRect.x - collisionRect.w;
						if(isVerticallyFlipped)
							collisionRect.y = info.tileSize.y - collisionRect.y - collisionRect.h;

						collisionRect.x += tileWorldPos.x; 
						collisionRect.y += tileWorldPos.y; 

						bool shouldBeAdded = true;
						for(FloatRect collisionDenialArea : mDenialAreas.collisionsAndLightWalls)
						{
							if(intersect(collisionDenialArea, collisionRect)) 
							{
								shouldBeAdded = false;
								break;
							}
						}

						if(shouldBeAdded)
						{
							for(FloatRect collisionDenialArea : mDenialAreas.collisions)
							{
								if(intersect(collisionDenialArea, collisionRect)) 
								{
									shouldBeAdded = false;
									break;
								}
							}

							if(shouldBeAdded)
								chunkCollisionRects.emplace_back(collisionRect);
						}

					}

					// circle collision bodies
					for(auto collisionCircle : tilesData.circleCollisions[i])
					{
						if(isHorizontallyFlipped)
							collisionCircle.offset.x = info.tileSize.x - collisionCircle.offset.x - collisionCircle.radius;
						if(isVerticallyFlipped)
							collisionCircle.offset.y = info.tileSize.y - collisionCircle.offset.y - collisionCircle.radius;

						collisionCircle.offset.x += tileWorldPos.x; 
						collisionCircle.offset.y += tileWorldPos.y; 

						auto collisionCircleRect = FloatRect(collisionCircle.offset, {collisionCircle.radius, collisionCircle.radius});

						bool shouldBeAdded = true;
						for(FloatRect collisionDenialArea : mDenialAreas.collisionsAndLightWalls)
						{
							if(intersect(collisionDenialArea, collisionCircleRect)) 
							{
								shouldBeAdded = false;
								break;
							}
						}

						if(shouldBeAdded)
						{
							for(FloatRect collisionDenialArea : mDenialAreas.collisions)
							{
								if(intersect(collisionDenialArea, collisionCircleRect)) 
								{
									shouldBeAdded = false;
									break;
								}
							}

							if(shouldBeAdded)
								chunkCollisionCircles.emplace_back(collisionCircle);
						}
					}

					// light walls
					for(FloatRect lightWallRect : tilesData.lightWalls[i])
					{
						if(isHorizontallyFlipped)
							lightWallRect.x = info.tileSize.x - lightWallRect.x - lightWallRect.w;
						if(isVerticallyFlipped)
							lightWallRect.y = info.tileSize.y - lightWallRect.y - lightWallRect.h;

						lightWallRect.x += tileWorldPos.x; 
						lightWallRect.y += tileWorldPos.y; 

						bool shouldBeAdded = true;
						for(FloatRect lightWallDenialArea : mDenialAreas.collisionsAndLightWalls)
						{
							if(intersect(lightWallDenialArea, lightWallRect)) 
							{
								shouldBeAdded = false;
								break;
							}
						}

						if(shouldBeAdded)
						{
							for(FloatRect lightWallDenialArea : mDenialAreas.lightWalls)
							{
								if(intersect(lightWallDenialArea, lightWallRect))
									shouldBeAdded = false;
							}

							if(shouldBeAdded)
								lightWalls.emplace_back(lightWallRect);
						}
					}

					// puzzle grid collisions
					bool road = tilesData.puzzleGridRoads[i];
					puzzleGridRoadChunk.tiles[chunkRelativePosInTiles.y][chunkRelativePosInTiles.x] = road;
					if(road) isThereAnyPuzzleGridRoadInThisChunk = true;

					// pits
					Pit pit = tilesData.pits[i];
					if(pit.exists)
					{
						pit.bounds.pos += tileWorldPos;
						pitChunk.pits.emplace_back(pit.bounds);
					}

					break;
				}
			}
		}
	}

	if(isThereAnyPuzzleGridRoadInThisChunk)
	{
		// create puzzle grid road chunk in registry
		auto intChunkPos = Cast<Vec2i>(chunkPos);
		auto entity = mGameRegistry->create();
		mGameRegistry->assign<component::PuzzleGridRoadChunk>(entity, puzzleGridRoadChunk);
		mGameRegistry->assign<component::PuzzleGridPos>(entity, intChunkPos);
		mAlreadyCreatedPuzzleGridRoadChunks.emplace_back(intChunkPos);
		#ifndef PH_DISTRIBUTION
		mGameRegistry->assign<component::DebugName>(entity, component::DebugName{"RoadChunk\0"});
		mGameRegistry->assign<component::BodyRect>(entity, FloatRect(chunkPos * 16.f, Vec2(12.f * 16.f)));
		mGameRegistry->assign<component::DebugColor>(entity, Random::generateColor(sf::Color(0, 0, 0, 50), sf::Color(255, 255, 255, 50))); 
		#endif
	}

	if(!pitChunk.pits.empty())
	{
		// create pit chunk in registry
		auto entity = mGameRegistry->create();
		mGameRegistry->assign<component::PitChunk>(entity, pitChunk);
		mGameRegistry->assign<component::BodyRect>(entity, FloatRect(chunkPos * 16.f, Vec2(sChunkSize * 16.f)));
		#ifndef PH_DISTRIBUTION
		mGameRegistry->assign<component::DebugName>(entity, component::DebugName{"PitChunk\0"});
		mGameRegistry->assign<component::DebugColor>(entity, Random::generateColor(sf::Color(0, 0, 0, 50), sf::Color(255, 255, 255, 50))); 
		#endif
	}

	if(!quads.empty())
	{
		// transform chunk bounds to world coords so we can later use them for culling in RenderSystem
		quadsBounds.x *= Cast<float>(info.tileSize.x);
		quadsBounds.y *= Cast<float>(info.tileSize.y);
		quadsBounds.w *= Cast<float>(info.tileSize.x);
		quadsBounds.h *= Cast<float>(info.tileSize.y);

		// check should we construct ground chunk or normal chunk 
		FloatRect groundTextureRect = quads[0].textureRect;
		bool groundChunkShouldBeConstructed = false;
		if(quads.size() == 144)
		{
			if(lightWalls.empty())
			{
				groundChunkShouldBeConstructed = true;
				for(auto& quad : quads)
				{
					if(quad.textureRect != groundTextureRect)
					{
						groundChunkShouldBeConstructed = false;
						break;
					}
				}
			}
		}

		// construct chunk in the registry
		if(groundChunkShouldBeConstructed)
		{
			auto groundChunkEntity = mTemplates->createCopy("GroundMapChunk", *mGameRegistry);

			auto& chunkBody = mGameRegistry->get<component::BodyRect>(groundChunkEntity);
			chunkBody = quadsBounds;

			auto& grc = mGameRegistry->get<component::GroundRenderChunk>(groundChunkEntity);
			grc.textureRect = groundTextureRect;
			grc.z = z;

			if(outdoor)
			{
				auto& ob = mGameRegistry->assign<component::OutdoorBlend>(groundChunkEntity);
				ob.brightness = 1.f;
			}
			else
			{
				auto& ib = mGameRegistry->assign<component::IndoorBlend>(groundChunkEntity);
				ib.alpha = 0.f;
			}
		}
		else
		{
			auto chunkEntity = mTemplates->createCopy("MapChunk", *mGameRegistry);

			auto& chunkBody = mGameRegistry->get<component::BodyRect>(chunkEntity);
			chunkBody = quadsBounds;

			auto& rc = mGameRegistry->get<component::RenderChunk>(chunkEntity);
			rc.quads = quads;
			rc.lightWalls = lightWalls;
			rc.z = z;
			rc.rendererID = Renderer::registerNewChunk(quadsBounds);

			if(outdoor)
			{
				auto& ob = mGameRegistry->assign<component::OutdoorBlend>(chunkEntity);
				ob.brightness = 1.f;
			}
			else
			{
				auto& ib = mGameRegistry->assign<component::IndoorBlend>(chunkEntity);
				ib.alpha = 0.f;
			}

			auto& mscb = mGameRegistry->get<component::MultiStaticCollisionBody>(chunkEntity);
			mscb.rects = chunkCollisionRects;
			mscb.circles = chunkCollisionCircles;
		}
	}
}

std::size_t XmlMapParser::findTilesetIndex(u32 globalTileId, const TilesetsData& tilesets) const
{
	for (size_t i = 0; i < tilesets.firstGlobalTileIds.size(); ++i) 
	{
		const u32 firstGlobalTileId = tilesets.firstGlobalTileIds[i];
		const u32 lastGlobalTileId = firstGlobalTileId + tilesets.tileCounts[i] - 1;
		if (globalTileId >= firstGlobalTileId && globalTileId <= lastGlobalTileId)
			return i;
	}
	return std::string::npos;
}

std::size_t XmlMapParser::findTilesIndex(u32 firstGlobalTileId, const std::vector<TilesData>& tilesData) const
{
	for (size_t i = 0; i < tilesData.size(); ++i)
		if (firstGlobalTileId == tilesData[i].firstGlobalTileId)
			return i;
	return std::string::npos;
}

}

