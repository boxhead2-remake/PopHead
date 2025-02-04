
// NOTE: This code is automatically generated by meta/entitiesParser.exe 

#include "pch.hpp"
#include "entitiesDebugger.hpp"
#include "Renderer/renderer.hpp"

#include "ECS/Components/aiComponents.hpp"
#include "ECS/Components/animationComponents.hpp"
#include "ECS/Components/audioComponents.hpp"
#include "ECS/Components/charactersComponents.hpp"
#include "ECS/Components/debugComponents.hpp"
#include "ECS/Components/graphicsComponents.hpp"
#include "ECS/Components/itemComponents.hpp"
#include "ECS/Components/objectsComponents.hpp"
#include "ECS/Components/particleComponents.hpp"
#include "ECS/Components/physicsComponents.hpp"
#include "ECS/Components/simRegionComponents.hpp"


extern bool debugWindowOpen;

namespace ph::system {

constexpr u32 lookForSize = 255;
static char lookFor[lookForSize];
static bool highlightSelected = true;

static char* components[] = {
	"Health", "Damage", "Player", "Killable", "InPlayerGunAttackArea"
};
static bool selectedComponents[IM_ARRAYSIZE(components)];

static bool selectingInWorldMode = false;
static float selectingInWorldModeF4InputDelay = 0.f;
static u32 selectingInWorldModeZPriority = 0;
static float selectingInWorldModeZPriorityInputDelay = 0.f;

EntitiesDebugger::EntitiesDebugger(entt::registry& reg, sf::Window* window)
	:System(reg)
	,mWindow(window)
{
}

static u32 getCharCount(char* str, size_t size)
{
	for(u32 charCount = 0; charCount < static_cast<u32>(size); ++charCount)
		if(str[charCount] == 0)
			return charCount;
	return static_cast<u32>(size);
}

using namespace component;

void EntitiesDebugger::update(float dt)
{
	PH_PROFILE_FUNCTION();	

	if(debugWindowOpen && ImGui::BeginTabItem("entities debugger"))
	{
		ImGui::BeginChild("entities", ImVec2(360, 0), true);
		ImGui::Checkbox("hightlight selected", &highlightSelected);

		if(selectingInWorldModeF4InputDelay > 0.f)
			selectingInWorldModeF4InputDelay -= dt;

		if(selectingInWorldMode)
		{
			if(ImGui::Button("leave world entity seleting [F4]"))
			{
				selectingInWorldMode = false;
			}
			else if(sf::Keyboard::isKeyPressed(sf::Keyboard::F4) && selectingInWorldModeF4InputDelay <= 0.f)
			{	
				selectingInWorldModeF4InputDelay = 0.4f;
				selectingInWorldMode = false;
			}
		}
		else
		{
			if(ImGui::Button("select entity in world [F4]"))
			{
				selectingInWorldMode = true;
			}
			else if(sf::Keyboard::isKeyPressed(sf::Keyboard::F4) && selectingInWorldModeF4InputDelay <= 0.f)
			{	
				selectingInWorldModeF4InputDelay = 0.4f;
				selectingInWorldMode = true;
			}
		}

		if(selectingInWorldMode)
		{
			if(selectingInWorldModeZPriorityInputDelay > 0.f)
				selectingInWorldModeZPriorityInputDelay -= dt;

			FloatRect currentCamBounds;
			mRegistry.view<Camera>().each([&]
			(auto camera)
			{
				if(camera.name == Camera::currentCameraName)
					currentCamBounds = camera.bounds;
			});

			auto mouseWindowPos = Cast<sf::Vector2f>(sf::Mouse::getPosition(*mWindow));
			auto resolutionRatio = hadamardDiv(currentCamBounds.size, Cast<sf::Vector2f>(mWindow->getSize()));
			auto mouseWorldPos = (hadamardMul(mouseWindowPos, resolutionRatio)) + currentCamBounds.pos; 

			struct EntityUnderCursor
			{
				entt::entity entity;
				u8 z;
			};
			std::vector<EntityUnderCursor> entitiesUnderCursor;

			auto bodiesView = mRegistry.view<BodyRect>();
			for(auto entity : bodiesView)
			{
				const auto& body = bodiesView.get<BodyRect>(entity); 
				if(body.contains(mouseWorldPos))
				{
					EntityUnderCursor euc;
					euc.entity = entity;

					if(auto* rq = mRegistry.try_get<RenderQuad>(entity))
						euc.z = rq->z; 
					else if(auto* rc = mRegistry.try_get<RenderChunk>(entity))
						euc.z = rc->z;
					else if(auto* grc = mRegistry.try_get<GroundRenderChunk>(entity))
						euc.z = grc->z;
					else
						euc.z = 255;

					entitiesUnderCursor.emplace_back(euc);
				}
			}

			std::sort(entitiesUnderCursor.begin(), entitiesUnderCursor.end(), []
			(const auto& a, const auto& b)
			{
				return a.z < b.z;
			});

			if(selectingInWorldModeZPriority > entitiesUnderCursor.size() - 1)
				selectingInWorldModeZPriority = 0; 

			ImGui::Text("left mouse button - select entity");
			ImGui::Text("right mouse button - leave world entity seleting");
			ImGui::Text("middle mouse button - change z priority");

			if(entitiesUnderCursor.size())
			{
				auto underCursorEntity = entitiesUnderCursor[selectingInWorldModeZPriority].entity;
				auto underCursorZ = entitiesUnderCursor[selectingInWorldModeZPriority].z;

				u8 alpha = underCursorEntity == mSelected ? 45 : 150;
				auto& body = mRegistry.get<BodyRect>(underCursorEntity);
				Renderer::submitQuad(Null, Null, &sf::Color(255, 0, 0, alpha), Null,
					body.pos, body.size, underCursorZ, 0.f, {}, ProjectionType::gameWorld, false);

				for(u32 i = 1; i < entitiesUnderCursor.size(); ++i)
				{
					auto& body = mRegistry.get<BodyRect>(underCursorEntity);
					Renderer::submitQuad(Null, Null, &sf::Color(255, 0, 0, 40), Null,
						body.pos, body.size, underCursorZ, 0.f, {}, ProjectionType::gameWorld, false);
				}

				if(sf::Mouse::isButtonPressed(sf::Mouse::Left))
				{
					mSelected = underCursorEntity; 
				}

				if(sf::Mouse::isButtonPressed(sf::Mouse::Middle) && selectingInWorldModeZPriorityInputDelay <= 0.f)
				{
					selectingInWorldModeZPriorityInputDelay = 0.2f;
					if(selectingInWorldModeZPriority < entitiesUnderCursor.size() - 1)
						++selectingInWorldModeZPriority;
					else
						selectingInWorldModeZPriority = 0;
				}
			}

			if(sf::Mouse::isButtonPressed(sf::Mouse::Right))
			{
				selectingInWorldMode = false;	
			}
		}
		else
		{
			selectingInWorldModeZPriorityInputDelay = 0.f;
		}

		/* TODO: Add support for components choosing
		if(ImGui::TreeNode("choose components"))
		{
			if(ImGui::ListBoxHeader("comListBox"))
			{	
				for(int i = 0; i < IM_ARRAYSIZE(components); ++i)			
				{
					ImGui::Selectable(components[i], selectedComponents + i);
				}
				ImGui::ListBoxFooter();
			}
			ImGui::TreePop();
		}
		*/

		ImGui::InputText("debug name", lookFor, lookForSize);

		u32 lookForCharCount = getCharCount(lookFor, lookForSize);

		auto selectableEntity = [=](entt::entity entity)
		{
			bool displayThisEntity = true;
			char label[50];
			if(auto* debugName = mRegistry.try_get<DebugName>(entity))
			{
				char* name = debugName->name;
				u32 nameCharCount = getCharCount(name, strlen(name));
				sprintf(label, "%u - %s", entity, name);
				if(lookForCharCount != 0 && lookFor[0] != ' ')
				{
					for(u32 i = 0; i <= nameCharCount && i < lookForCharCount; ++i)
					{
						char nameChar = name[i];
						char lookForChar = lookFor[i]; 
						if(nameChar != lookForChar)
						{
							if(lookForChar > 96 && lookForChar < 123)
							{
								if(lookForChar - 32 != nameChar)
								{
									displayThisEntity = false;
									break;
								}
							}
							else
							{
								displayThisEntity = false;
								break;
							}
						}
					}
				}
			}
			else if(lookFor[0] == ' ')
			{
				sprintf(label, "%u", entity);
			}
			else
			{
				displayThisEntity = false;
			}

			if(displayThisEntity && ImGui::Selectable(label, mSelected == entity))
			{
				mSelected = entity;
			}
		};

		std::vector<entt::component> types;

		/* TODO: Add support for components choosing
		if(selectedComponents[0]) types.emplace_back(mRegistry.type<Health>());
		if(selectedComponents[1]) types.emplace_back(mRegistry.type<Damage>());
		if(selectedComponents[2]) types.emplace_back(mRegistry.type<Player>());
		if(selectedComponents[3]) types.emplace_back(mRegistry.type<Killable>());
		if(selectedComponents[4]) types.emplace_back(mRegistry.type<InPlayerGunAttackArea>());

		if(types.empty() || types.size() == IM_ARRAYSIZE(components))
		{
			mRegistry.each([=](auto entity)
			{
				selectableEntity(entity);
			});
		}
		else
		{
			mRegistry.runtime_view(types.cbegin(), types.cend()).each([=](auto entity)
			{
				selectableEntity(entity);
			});	
		}
		*/

		ImGui::BeginChild("entities2", ImVec2(0, 0), true);
			mRegistry.each([=](auto entity)
			{
				selectableEntity(entity);
			});
		ImGui::EndChild();

		ImGui::EndChild();
		ImGui::SameLine();

		ImGui::BeginChild("components view");

		if(auto* debugName = mRegistry.try_get<DebugName>(mSelected))
			ImGui::Text("%s Components view:", debugName->name);
		else
			ImGui::Text("Components view:");

		if(mRegistry.valid(mSelected))
		{
			bool bodyValid = false;
			BodyRect body;	

			// NOTE: Change value to 0 if this code doesn't compile
			//       because of bug in components parser!
			//       If that happens you can make a commit with #if value set to 0
			//       Make sure to inform Czapa about bug in parser. 
			#if 1

			// HEADER ENDS

if(auto* c = mRegistry.try_get<Zombie>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("Zombie");
ImGui::Text("pathMode: PathMode view is not supported!");
ImGui::Text("currentDirectionVector: %f, %f", c->currentDirectionVector.x, c->currentDirectionVector.y);
ImGui::Text("timeFromStartingThisMove: %f", c->timeFromStartingThisMove);
ImGui::Text("timeFromLastGrowl: %f", c->timeFromLastGrowl);
ImGui::Text("timeToMoveToAnotherTile: %f", c->timeToMoveToAnotherTile);
}
if(auto* c = mRegistry.try_get<SlowZombieBehavior>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("SlowZombieBehavior");
ImGui::Text("coolDownTimer: %f", c->coolDownTimer);
}
if(auto* c = mRegistry.try_get<Health>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("Health");
ImGui::Text("healthPoints: %i", c->healthPoints);
ImGui::Text("maxHealthPoints: %i", c->maxHealthPoints);
}
if(auto* c = mRegistry.try_get<Damage>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("Damage");
ImGui::Text("damageDealt: %i", c->damageDealt);
}
if(mRegistry.has<Player>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("Player");
}
if(mRegistry.has<Killable>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("Killable");
}
if(mRegistry.has<CurrentlyDashing>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("CurrentlyDashing");
}
if(mRegistry.has<InPlayerGunAttackArea>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("InPlayerGunAttackArea");
}
if(mRegistry.has<IsOnPlatform>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("IsOnPlatform");
}
if(auto* c = mRegistry.try_get<FallingIntoPit>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("FallingIntoPit");
ImGui::Text("timeToEnd: %f", c->timeToEnd);
}
if(auto* c = mRegistry.try_get<FaceDirection>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("FaceDirection: %f, %f", c->x, c->y);
}
if(mRegistry.has<TaggedToDestroy>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("TaggedToDestroy");
}
if(auto* c = mRegistry.try_get<CollisionWithPlayer>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("CollisionWithPlayer");
ImGui::Text("pushForce: %f", c->pushForce);
if(c->isCollision) ImGui::Text("isCollision: true"); else ImGui::Text("isCollision: false");
}
if(auto* c = mRegistry.try_get<Lifetime>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("Lifetime");
ImGui::Text("lifetime: %f", c->lifetime);
}
if(mRegistry.has<CurrentGun>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("CurrentGun");
}
if(mRegistry.has<CurrentMeleeWeapon>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("CurrentMeleeWeapon");
}
if(auto* c = mRegistry.try_get<TeleportPoint>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("TeleportPoint");
ImGui::Text("name: %s", c->name.c_str());
}
if(auto* c = mRegistry.try_get<DebugColor>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("DebugColor"); // unknown parent type!
}
if(auto* c = mRegistry.try_get<TextureRect>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("TextureRect: %i, %i, %i, %i", c->x, c->y, c->w, c->h);
}
if(auto* c = mRegistry.try_get<IndoorOutdoorBlendArea>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("IndoorOutdoorBlendArea");
switch(c->exit) {
case IndoorOutdoorBlendArea::Left: ImGui::Text("exit: Left"); break;
case IndoorOutdoorBlendArea::Right: ImGui::Text("exit: Right"); break;
case IndoorOutdoorBlendArea::Top: ImGui::Text("exit: Top"); break;
case IndoorOutdoorBlendArea::Down: ImGui::Text("exit: Down"); break;
default: ImGui::Text("IndoorOutdoorBlendArea: unknown enumeration!!!");
}
}
if(mRegistry.has<IndoorArea>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("IndoorArea");
}
if(mRegistry.has<OutdoorArea>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("OutdoorArea");
}
if(auto* c = mRegistry.try_get<IndoorOutdoorBlend>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("IndoorOutdoorBlend");
ImGui::Text("outdoor: %f", c->outdoor);
ImGui::Text("brightness: %f", c->brightness);
ImGui::Text("alpha: %f", c->alpha);
}
if(auto* c = mRegistry.try_get<OutdoorBlend>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("OutdoorBlend");
ImGui::Text("brightness: %f", c->brightness);
}
if(auto* c = mRegistry.try_get<IndoorBlend>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("IndoorBlend");
ImGui::Text("alpha: %f", c->alpha);
}
if(auto* c = mRegistry.try_get<RenderQuad>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("RenderQuad");
ImGui::Text("texture: %p", c->texture);
ImGui::Text("shader: %p", c->shader);
ImGui::Text("rotationOrigin: %f, %f", c->rotationOrigin.x, c->rotationOrigin.y);
ImGui::Text("color: %u, %u, %u, %u", c->color.r, c->color.g, c->color.b, c->color.a);
ImGui::Text("rotation: %f", c->rotation);
ImGui::Text("z: %u", c->z);
}
if(auto* c = mRegistry.try_get<GroundRenderChunk>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("GroundRenderChunk");
ImGui::Text("textureRect: %f, %f, %f, %f", c->textureRect.x,  c->textureRect.y, c->textureRect.w, c->textureRect.h);
ImGui::Text("z: %u", c->z);
if(c->outdoor) ImGui::Text("outdoor: true"); else ImGui::Text("outdoor: false");
}
if(auto* c = mRegistry.try_get<RenderChunk>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("RenderChunk");
ImGui::Text("quads: std::vector view is not supported!");
ImGui::Text("lightWalls: std::vector view is not supported!");
ImGui::Text("rendererID: %u", c->rendererID);
ImGui::Text("z: %u", c->z);
if(c->outdoor) ImGui::Text("outdoor: true"); else ImGui::Text("outdoor: false");
}
if(auto* c = mRegistry.try_get<LightWall>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("LightWall: %f, %f, %f, %f", c->x, c->y, c->w, c->h);
}
if(auto* c = mRegistry.try_get<LightSource>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("LightSource");
ImGui::Text("offset: %f, %f", c->offset.x, c->offset.y);
ImGui::Text("color: %u, %u, %u, %u", c->color.r, c->color.g, c->color.b, c->color.a);
ImGui::Text("attenuationAddition: %f", c->attenuationAddition);
ImGui::Text("attenuationFactor: %f", c->attenuationFactor);
ImGui::Text("attenuationSquareFactor: %f", c->attenuationSquareFactor);
ImGui::Text("startAngle: %f", c->startAngle);
ImGui::Text("endAngle: %f", c->endAngle);
if(c->rayCollisionDetection) ImGui::Text("rayCollisionDetection: true"); else ImGui::Text("rayCollisionDetection: false");
}
if(auto* c = mRegistry.try_get<CameraRoom>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("CameraRoom");
ImGui::Text("timeFromPlayerEntrance: %f", c->timeFromPlayerEntrance);
ImGui::Text("edgeAreaSize: %f", c->edgeAreaSize);
ImGui::Text("to: from view is not supported!");
if(c->playerWasInCenter) ImGui::Text("playerWasInCenter: true"); else ImGui::Text("playerWasInCenter: false");
}
if(auto* c = mRegistry.try_get<Camera>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("Camera");
ImGui::Text("name: %s", c->name.c_str());
ImGui::Text("bounds: %f, %f, %f, %f", c->bounds.x,  c->bounds.y, c->bounds.w, c->bounds.h);
}
if(mRegistry.has<DebugCamera>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("DebugCamera");
}
if(mRegistry.has<HiddenForRenderer>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("HiddenForRenderer");
}
if(auto* c = mRegistry.try_get<Medkit>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("Medkit");
ImGui::Text("addHealthPoints: %i", c->addHealthPoints);
}
if(auto* c = mRegistry.try_get<Bullets>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("Bullets");
ImGui::Text("numOfPistolBullets: %i", c->numOfPistolBullets);
ImGui::Text("numOfShotgunBullets: %i", c->numOfShotgunBullets);
}
if(mRegistry.has<BulletBox>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("BulletBox");
}
if(auto* c = mRegistry.try_get<AreaVelocityChangingEffect>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("AreaVelocityChangingEffect");
ImGui::Text("areaSpeedMultiplier: %f", c->areaSpeedMultiplier);
}
if(auto* c = mRegistry.try_get<Hint>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("Hint");
ImGui::Text("hintName: %s", c->hintName.c_str());
ImGui::Text("keyboardContent: %s", c->keyboardContent.c_str());
ImGui::Text("joystickContent: %s", c->joystickContent.c_str());
if(c->isShown) ImGui::Text("isShown: true"); else ImGui::Text("isShown: false");
}
if(auto* c = mRegistry.try_get<PushingArea>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("PushingArea");
ImGui::Text("pushForce: %f, %f", c->pushForce.x, c->pushForce.y);
}
if(auto* c = mRegistry.try_get<Puzzle>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("Puzzle");
ImGui::Text("id: %u", c->id);
}
if(auto* c = mRegistry.try_get<PuzzleId>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("PuzzleId");
ImGui::Text("struct: union view is not supported!");
ImGui::Text("puzzleId: %u", c->puzzleId);
ImGui::Text("elementId: %u", c->elementId);
}
if(auto* c = mRegistry.try_get<PuzzleColor>(mSelected))
{
ImGui::Separator();
switch(*c)
{
case PuzzleColor::Grey: ImGui::BulletText("PuzzleColor: Grey"); break;
case PuzzleColor::Red: ImGui::BulletText("PuzzleColor: Red"); break;
case PuzzleColor::Green: ImGui::BulletText("PuzzleColor: Green"); break;
case PuzzleColor::Blue: ImGui::BulletText("PuzzleColor: Blue"); break;
default: ImGui::BulletText("PuzzleColor: unknown enumeration!!!");
}
}
if(auto* c = mRegistry.try_get<Lever>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("Lever");
if(c->active) ImGui::Text("active: true"); else ImGui::Text("active: false");
if(c->wasJustSwitched) ImGui::Text("wasJustSwitched: true"); else ImGui::Text("wasJustSwitched: false");
if(c->turnOffAfterSwitch) ImGui::Text("turnOffAfterSwitch: true"); else ImGui::Text("turnOffAfterSwitch: false");
}
if(auto* c = mRegistry.try_get<PressurePlate>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("PressurePlate");
ImGui::Text("pressedByColor: PuzzleColor view is not supported!");
if(c->isPressed) ImGui::Text("isPressed: true"); else ImGui::Text("isPressed: false");
if(c->isPressIrreversible) ImGui::Text("isPressIrreversible: true"); else ImGui::Text("isPressIrreversible: false");
}
if(auto* c = mRegistry.try_get<PuzzleBoulder>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("PuzzleBoulder");
ImGui::Text("pushedLeftSince: %f", c->pushedLeftSince);
ImGui::Text("pushedRightSince: %f", c->pushedRightSince);
ImGui::Text("pushedUpSince: %f", c->pushedUpSince);
ImGui::Text("pushedDownSince: %f", c->pushedDownSince);
ImGui::Text("movingLeft: %f", c->movingLeft);
ImGui::Text("movingRight: %f", c->movingRight);
ImGui::Text("movingUp: %f", c->movingUp);
ImGui::Text("movingDown: %f", c->movingDown);
if(c->movedGridPosInThisMove) ImGui::Text("movedGridPosInThisMove: true"); else ImGui::Text("movedGridPosInThisMove: false");
}
if(auto* c = mRegistry.try_get<PuzzleGridPos>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("PuzzleGridPos: %i, %i", c->x, c->y);
}
if(auto* c = mRegistry.try_get<PuzzleGridRoadChunk>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("PuzzleGridRoadChunk");
if(c->tiles) ImGui::Text("tiles: true"); else ImGui::Text("tiles: false");
ImGui::Text("collision: road view is not supported!");
}
if(auto* c = mRegistry.try_get<PitChunk>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("PitChunk");
ImGui::Text("pits: std::vector view is not supported!");
}
if(auto* c = mRegistry.try_get<Gate>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("Gate");
ImGui::Text("id: %u", c->id);
if(c->previouslyOpen) ImGui::Text("previouslyOpen: true"); else ImGui::Text("previouslyOpen: false");
if(c->open) ImGui::Text("open: true"); else ImGui::Text("open: false");
}
if(auto* c = mRegistry.try_get<Spikes>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("Spikes");
ImGui::Text("timeToChange: %f", c->timeToChange);
ImGui::Text("changeFrequency: %f", c->changeFrequency);
if(c->changes) ImGui::Text("changes: true"); else ImGui::Text("changes: false");
if(c->active) ImGui::Text("active: true"); else ImGui::Text("active: false");
}
if(auto* c = mRegistry.try_get<MovingPlatform>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("MovingPlatform");
ImGui::Text("pathBody: %f, %f, %f, %f", c->pathBody.x,  c->pathBody.y, c->pathBody.w, c->pathBody.h);
ImGui::Text("fullVelocity: %f, %f", c->fullVelocity.x, c->fullVelocity.y);
ImGui::Text("currentVelocity: %f, %f", c->currentVelocity.x, c->currentVelocity.y);
ImGui::Text("pathCompletion: %f", c->pathCompletion);
if(c->active) ImGui::Text("active: true"); else ImGui::Text("active: false");
}
if(auto* c = mRegistry.try_get<FallingPlatform>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("FallingPlatform");
switch(c->state) {
case FallingPlatform::isStable: ImGui::Text("state: isStable"); break;
case FallingPlatform::isFallingApart: ImGui::Text("state: isFallingApart"); break;
case FallingPlatform::isRecovering: ImGui::Text("state: isRecovering"); break;
default: ImGui::Text("FallingPlatform: unknown enumeration!!!");
}
ImGui::Text("timeToChangeState: %f", c->timeToChangeState);
ImGui::Text("timeToChangeAnimationFrame: %f", c->timeToChangeAnimationFrame);
}
if(mRegistry.has<Weather>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("Weather");
}
if(mRegistry.has<WeatherArea>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("WeatherArea");
}
if(auto* c = mRegistry.try_get<WeatherType>(mSelected))
{
ImGui::Separator();
switch(*c)
{
case WeatherType::Sunny: ImGui::BulletText("WeatherType: Sunny"); break;
case WeatherType::Cave: ImGui::BulletText("WeatherType: Cave"); break;
case WeatherType::DrizzleRain: ImGui::BulletText("WeatherType: DrizzleRain"); break;
case WeatherType::NormalRain: ImGui::BulletText("WeatherType: NormalRain"); break;
case WeatherType::HeavyRain: ImGui::BulletText("WeatherType: HeavyRain"); break;
default: ImGui::BulletText("WeatherType: unknown enumeration!!!");
}
}
if(auto* c = mRegistry.try_get<SavePoint>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("SavePoint");
if(c->isIntersectingPlayer) ImGui::Text("isIntersectingPlayer: true"); else ImGui::Text("isIntersectingPlayer: false");
ImGui::Text("timeSincePlayerSteppedOnIt: %f", c->timeSincePlayerSteppedOnIt);
}
if(auto* c = mRegistry.try_get<ParticleEmitter>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("ParticleEmitter");
ImGui::Text("particles: std::vector view is not supported!");
ImGui::Text("parTexture: %p", c->parTexture);
ImGui::Text("spawnPositionOffset: %f, %f", c->spawnPositionOffset.x, c->spawnPositionOffset.y);
ImGui::Text("randomSpawnAreaSize: %f, %f", c->randomSpawnAreaSize.x, c->randomSpawnAreaSize.y);
ImGui::Text("parInitialVelocity: %f, %f", c->parInitialVelocity.x, c->parInitialVelocity.y);
ImGui::Text("parInitialVelocityRandom: %f, %f", c->parInitialVelocityRandom.x, c->parInitialVelocityRandom.y);
ImGui::Text("parAcceleration: %f, %f", c->parAcceleration.x, c->parAcceleration.y);
ImGui::Text("parSize: %f, %f", c->parSize.x, c->parSize.y);
ImGui::Text("parStartColor: %u, %u, %u, %u", c->parStartColor.r, c->parStartColor.g, c->parStartColor.b, c->parStartColor.a);
ImGui::Text("parEndColor: %u, %u, %u, %u", c->parEndColor.r, c->parEndColor.g, c->parEndColor.b, c->parEndColor.a);
ImGui::Text("parWholeLifetime: %f", c->parWholeLifetime);
ImGui::Text("amountOfParticles: %u", c->amountOfParticles);
ImGui::Text("amountOfAlreadySpawnParticles: %u", c->amountOfAlreadySpawnParticles);
ImGui::Text("parZ: %u", c->parZ);
if(c->oneShot) ImGui::Text("oneShot: true"); else ImGui::Text("oneShot: false");
if(c->isEmitting) ImGui::Text("isEmitting: true"); else ImGui::Text("isEmitting: false");
if(c->wasInitialized) ImGui::Text("wasInitialized: true"); else ImGui::Text("wasInitialized: false");
}
if(auto* c = mRegistry.try_get<MultiParticleEmitter>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("MultiParticleEmitter");
ImGui::Text("particleEmitters: std::vector view is not supported!");
}
if(auto* c = mRegistry.try_get<BodyRect>(mSelected)) 
{
ImGui::Separator();
body = *c;
bodyValid = true;
ImGui::BulletText("BodyRect: %f, %f, %f, %f", c->x, c->y, c->w, c->h);
}
if(auto* c = mRegistry.try_get<BodyCircle>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("BodyCircle");
ImGui::Text("offset: %f, %f", c->offset.x, c->offset.y);
ImGui::Text("radius: %f", c->radius);
}
if(auto* c = mRegistry.try_get<Kinematics>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("Kinematics");
ImGui::Text("vel: %f, %f", c->vel.x, c->vel.y);
ImGui::Text("acceleration: %f, %f", c->acceleration.x, c->acceleration.y);
ImGui::Text("friction: %f", c->friction);
ImGui::Text("defaultFriction: %f", c->defaultFriction);
ImGui::Text("frictionLerpSpeed: %f", c->frictionLerpSpeed);
}
if(auto* c = mRegistry.try_get<CharacterSpeed>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("CharacterSpeed");
ImGui::Text("speed: %f", c->speed);
}
if(mRegistry.has<StaticCollisionBody>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("StaticCollisionBody");
}
if(auto* c = mRegistry.try_get<MultiStaticCollisionBody>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("MultiStaticCollisionBody");
ImGui::Text("rects: std::vector view is not supported!");
ImGui::Text("circles: std::vector view is not supported!");
}
if(auto* c = mRegistry.try_get<KinematicCollisionBody>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("KinematicCollisionBody");
ImGui::Text("mass: %f", c->mass);
if(c->staticallyMovedUp) ImGui::Text("staticallyMovedUp: true"); else ImGui::Text("staticallyMovedUp: false");
if(c->staticallyMovedDown) ImGui::Text("staticallyMovedDown: true"); else ImGui::Text("staticallyMovedDown: false");
if(c->staticallyMovedLeft) ImGui::Text("staticallyMovedLeft: true"); else ImGui::Text("staticallyMovedLeft: false");
if(c->staticallyMovedRight) ImGui::Text("staticallyMovedRight: true"); else ImGui::Text("staticallyMovedRight: false");
}
if(mRegistry.has<InsideSimRegion>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("InsideSimRegion");
}
if(mRegistry.has<DontCareAboutSimRegion>(mSelected)) 
{
ImGui::Separator();
ImGui::BulletText("DontCareAboutSimRegion");
}


			// FOOTER STARTS

			#endif // !COMPONENTS_PARSER_MESSED_UP

			if(highlightSelected && bodyValid)
			{
				Renderer::submitQuad(Null, Null, &sf::Color(255, 0, 0, 150), Null, body.pos, body.size,
									 10, 0.f, {}, ProjectionType::gameWorld, false);
			}
		}

		ImGui::EndChild();
		ImGui::EndTabItem();
	}
	else
	{
		selectingInWorldMode = false;
	}
}

}
