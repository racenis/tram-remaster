#include <framework/core.h>
#include <framework/logging.h>
#include <framework/ui.h>
#include <framework/gui.h>
#include <framework/async.h>
#include <framework/event.h>
#include <framework/message.h>
#include <framework/system.h>
#include <framework/stats.h>
#include <framework/worldcell.h>
#include <framework/language.h>
#include <framework/path.h>
#include <audio/audio.h>
#include <audio/sound.h>
#include <render/render.h>
#include <render/material.h>
#include <render/api.h>
#include <render/scene.h>
#include <physics/physics.h>
#include <entities/player.h>
#include <entities/staticworldobject.h>
#include <entities/light.h>
#include <entities/crate.h>
#include <entities/marker.h>
#include <entities/trigger.h>
#include <entities/decoration.h>
#include <components/player.h>
#include <components/animation.h>
#include <components/controller.h>
#include <components/render.h>
#include <components/trigger.h>
#include <extensions/camera/firstperson.h>
#include <extensions/menu/menu.h>
#include <extensions/kitchensink/kitchensink.h>
#include <extensions/kitchensink/entities.h>
#include <extensions/kitchensink/soundtable.h>

using namespace tram;
using namespace tram::UI;
using namespace tram::Render;
using namespace tram::Physics;
using namespace tram::Ext::Kitchensink;

enum {
	DIALOG_NONE,
	
	BINGUS_HELLO,
	BINGUS_WHATUP,
	BINUGS_WHERE_PEE,
	BINUGS_PEE_OK,
	BINUGS_HERE_MILK,
	
	FLOPPA_HELLO,
	FLOPPA_WHATUP,
	FLOPPA_WHATIS,
	FLOPPA_OKE,
	FLOPPA_DIDIT,
	FLOPPA_WASOKE,
	FLOPPA_WASNOTOKE,
	FLOPPA_WASMEH,
	FLOPPA_WHEREPEE,
	
	BINGUS_GIVEMORE,
	BINGUS_DONEDONE
};




int dialog = DIALOG_NONE;

int collected_milk = 0;
bool accept_milk_quest = false;
bool finish_milk_quest = false;

bool started_tramming = false;
bool finished_tramming = false;
static double time_started_tramming = -1;

bool asked_for_more = false;

bool floppa_whatupped = false;
bool floppa_agreed = false;
bool floppa_finished = false;

class LogicController : public Entity {
public:
    LogicController(name_t name) : Entity(name) {}
    void UpdateParameters() {}
    void SetParameters() {}
    void Load() {}
    void Unload() {}
    void Serialize() {}
    name_t GetType() { return "none"; }
    void MessageHandler(Message& msg) {
        name_t trigger = *(Value*)msg.data;
        std::cout << name << " triggered " << trigger << std::endl;
        
		if (trigger == "bingus") {
			dialog = BINGUS_HELLO;
			
			UI::SetInputState(UI::STATE_MENU_OPEN);
		}
		
		if (trigger == "floppa") {
			dialog = FLOPPA_HELLO;
			
			UI::SetInputState(UI::STATE_MENU_OPEN);
		}
		
		if (trigger == "tram") {
			started_tramming = true;
			time_started_tramming = GetTickTime();
		}
		
		if (trigger == "milk") {
			collected_milk++;
		}
		
		// TODO: implement
		
		// 1. start bingus conversation
		// 2. start floppa conversation 
		// 3. start tram ride
		// 4. trigger mongus
		
    }
};



static PathFollower* tram_follower_a = nullptr;
static PathFollower* tram_follower_b = nullptr;

static Player* player = nullptr;

static float BOB_WEIGHT = 1.0f;
static float TILT_WEIGHT = 1.0f;

Component<RenderComponent> skybox;

Ext::Camera::FirstPersonCamera* camera = nullptr;
void main_loop();

int main() {
	//SetSystemLoggingSeverity(System::SYSTEM_PLATFORM, SEVERITY_WARNING);

	StaticWorldObject::Register();
	Light::Register();
	Crate::Register();
	Marker::Register();
	Trigger::Register();
	tram::Decoration::Register();
	Ext::Kitchensink::Button::Register();

	Core::Init();
	UI::Init();
	
	//Render::API::RegisterShader(VERTEX_STATIC, RegisterMaterialType("poopwater"), "poopwater_static", "normal_static");
	//Render::API::RegisterShader(VERTEX_STATIC, RegisterMaterialType("bright"), "normal_static", "bright_static");
	
	/*Render::API::RegisterShader(VERTEX_STATIC, RegisterMaterialType("poopwater"), "normal_static", "normal_static");
	Render::API::RegisterShader(VERTEX_STATIC, RegisterMaterialType("bright"), "normal_static", "normal_static");*/
	
	Render::Init();
	Physics::Init();
#ifdef __EMSCRIPTEN__
	Async::Init(0);
#else
	Async::Init();
#endif
	Audio::Init();
	GUI::Init();

	Ext::Menu::Init();
	Ext::Camera::Init();
	Ext::Kitchensink::Init();

	Material::LoadMaterialInfo("material");
	//Language::Load("english");
	Language::Load("lv");
	
	Animation::Find("mongus-sway")->Load();
	Animation::Find("floppa-idle")->Load();
	Animation::Find("bingus-idle")->Load();
	
	Path::Find("tram-a")->LoadFromDisk();
	Path::Find("tram-b")->LoadFromDisk();
	
	/*Animation::Find("LinuxIdle")->Load();
	Animation::Find("LinuxCrash")->Load();
	Animation::Find("LinuxRetreat")->Load();
	Animation::Find("RatIdle")->Load();
	Animation::Find("RatDead")->Load();
	Animation::Find("RatAttack")->Load();
	Animation::Find("RatInquire")->Load();
	Animation::Find("GoblinIdle")->Load();
	Animation::Find("GoblinDead")->Load();
	Animation::Find("GoblinWalk")->Load();
	Animation::Find("GoblinFlinch")->Load();
	Animation::Find("GoblinAttack")->Load();
	Animation::Find("GoblinInquire")->Load();*/
	
	Audio::Sound::Find("metalhit1")->Load();
	Audio::Sound::Find("metalhit2")->Load();
	Audio::Sound::Find("metalhit3")->Load();
	Audio::Sound::Find("metalhit4")->Load();
	Audio::Sound::Find("metalstep1")->Load();
	Audio::Sound::Find("metalstep2")->Load();
	Audio::Sound::Find("metalstep3")->Load();
	Audio::Sound::Find("woodhit1")->Load();
	Audio::Sound::Find("woodhit2")->Load();
	Audio::Sound::Find("woodhit3")->Load();
	Audio::Sound::Find("splash1")->Load();
	Audio::Sound::Find("splash2")->Load();
	Audio::Sound::Find("splash3")->Load();
	Audio::Sound::Find("concretehit1")->Load();
	Audio::Sound::Find("concretehit2")->Load();
	Audio::Sound::Find("concretehit3")->Load();
	Audio::Sound::Find("concretestep1")->Load();
	Audio::Sound::Find("concretestep2")->Load();
	Audio::Sound::Find("concretestep3")->Load();
	Audio::Sound::Find("slide1")->Load();
	Audio::Sound::Find("slide2")->Load();
	Audio::Sound::Find("metalsqueak1")->Load();
	Audio::Sound::Find("metalsqueak2")->Load();
	Audio::Sound::Find("metalsqueak3")->Load();
	Audio::Sound::Find("metalrattle1")->Load();
	Audio::Sound::Find("pickup1")->Load();
	Audio::Sound::Find("nailgun1")->Load();
	Audio::Sound::Find("nailgun2")->Load();
	Audio::Sound::Find("locked1")->Load();
	Audio::Sound::Find("snorkle1")->Load();
	Audio::Sound::Find("snorkle2")->Load();
	Audio::Sound::Find("snorkle3")->Load();
	
	SoundTable* metal_sounds = SoundTable::Find("metal");
	metal_sounds->AddSound(SOUND_HIT, "metalhit1");
	metal_sounds->AddSound(SOUND_HIT, "metalhit2");
	metal_sounds->AddSound(SOUND_WALK_ON, "metalstep1");
	metal_sounds->AddSound(SOUND_WALK_ON, "metalstep2");
	metal_sounds->AddSound(SOUND_WALK_ON, "metalstep3");
	
	SoundTable* wood_sounds = SoundTable::Find("wood");
	wood_sounds->AddSound(SOUND_HIT, "woodhit1");
	wood_sounds->AddSound(SOUND_HIT, "woodhit2");
	wood_sounds->AddSound(SOUND_HIT, "woodhit3");
	wood_sounds->AddSound(SOUND_WALK_ON, "woodhit1");
	wood_sounds->AddSound(SOUND_WALK_ON, "woodhit2");
	wood_sounds->AddSound(SOUND_WALK_ON, "woodhit3");
	
	SoundTable* water_sounds = SoundTable::Find("water");
	water_sounds->AddSound(SOUND_HIT, "splash1");
	water_sounds->AddSound(SOUND_HIT, "splash2");
	water_sounds->AddSound(SOUND_HIT, "splash3");
	water_sounds->AddSound(SOUND_WALK_ON, "splash1");
	water_sounds->AddSound(SOUND_WALK_ON, "splash2");
	water_sounds->AddSound(SOUND_WALK_ON, "splash3");
	
	SoundTable* concrete_sounds = SoundTable::Find("concrete");
	concrete_sounds->AddSound(SOUND_HIT, "concretehit1");
	concrete_sounds->AddSound(SOUND_HIT, "concretehit2");
	concrete_sounds->AddSound(SOUND_HIT, "concretehit2");
	concrete_sounds->AddSound(SOUND_WALK_ON, "concretestep1");
	concrete_sounds->AddSound(SOUND_WALK_ON, "concretestep2");
	concrete_sounds->AddSound(SOUND_WALK_ON, "concretestep3");
	
	SoundTable* button_sounds = SoundTable::Find("button");
	button_sounds->AddSound(SOUND_OPEN, "slide1");
	button_sounds->AddSound(SOUND_OPEN, "slide2");
	button_sounds->AddSound(SOUND_CLOSE, "slide1");
	button_sounds->AddSound(SOUND_CLOSE, "slide2");
	//button_sounds->AddSound(SOUND_OPEN, "slide1");
	//button_sounds->AddSound(SOUND_CLOSE, "slide1");
	//button_sounds->AddSound(SOUND_END_OPEN, "slide1");
	//button_sounds->AddSound(SOUND_END_CLOSE, "slide1");
	
	SoundTable* gate_sounds = SoundTable::Find("gate");
	gate_sounds->AddSound(SOUND_LOCKED, "locked1");
	gate_sounds->AddSound(SOUND_LOCKED, "locked1");
	gate_sounds->AddSound(SOUND_OPEN, "metalsqueak1");
	gate_sounds->AddSound(SOUND_OPEN, "metalsqueak2");
	gate_sounds->AddSound(SOUND_OPEN, "metalsqueak3");
	gate_sounds->AddSound(SOUND_CLOSE, "metalsqueak1");
	gate_sounds->AddSound(SOUND_CLOSE, "metalsqueak2");
	gate_sounds->AddSound(SOUND_CLOSE, "metalsqueak3");
	gate_sounds->AddSound(SOUND_END_OPEN, "metalhit3");
	gate_sounds->AddSound(SOUND_END_OPEN, "metalhit4");
	gate_sounds->AddSound(SOUND_END_CLOSE, "metalhit3");
	gate_sounds->AddSound(SOUND_END_CLOSE, "metalhit4");
	
	SoundTable* door_sounds = SoundTable::Find("door");
	door_sounds->AddSound(SOUND_LOCKED, "locked1");
	door_sounds->AddSound(SOUND_LOCKED, "locked1");
	door_sounds->AddSound(SOUND_OPEN, "metalsqueak1");
	door_sounds->AddSound(SOUND_OPEN, "metalsqueak2");
	door_sounds->AddSound(SOUND_OPEN, "metalsqueak3");
	door_sounds->AddSound(SOUND_CLOSE, "metalsqueak1");
	door_sounds->AddSound(SOUND_CLOSE, "metalsqueak2");
	door_sounds->AddSound(SOUND_CLOSE, "metalsqueak3");
	door_sounds->AddSound(SOUND_END_OPEN, "metalhit1");
	door_sounds->AddSound(SOUND_END_OPEN, "metalhit2");
	door_sounds->AddSound(SOUND_END_CLOSE, "metalhit1");
	door_sounds->AddSound(SOUND_END_CLOSE, "metalhit2");
	
	SoundTable* lift_sounds = SoundTable::Find("lift");
	lift_sounds->AddSound(SOUND_OPEN, "metalrattle1");
	lift_sounds->AddSound(SOUND_OPEN, "metalrattle1");
	lift_sounds->AddSound(SOUND_CLOSE, "metalrattle1");
	lift_sounds->AddSound(SOUND_CLOSE, "metalrattle1");
	lift_sounds->AddSound(SOUND_END_OPEN, "metalhit3");
	lift_sounds->AddSound(SOUND_END_OPEN, "metalhit4");
	lift_sounds->AddSound(SOUND_END_CLOSE, "metalhit3");
	lift_sounds->AddSound(SOUND_END_CLOSE, "metalhit4");
	
	SoundTable* nailgun_sounds = SoundTable::Find("nailgun");
	nailgun_sounds->AddSound(SOUND_HIT, "nailgun1");
	nailgun_sounds->AddSound(SOUND_HIT, "nailgun2");
	
	SoundTable* snorkle_sounds = SoundTable::Find("snorkle");
	snorkle_sounds->AddSound(SOUND_HIT, "snorkle1");
	snorkle_sounds->AddSound(SOUND_HIT, "snorkle2");
	snorkle_sounds->AddSound(SOUND_HIT, "snorkle3");
	
	skybox.make();
	skybox->SetLightmap("fullbright");
    skybox->SetDirectionaLight(false);
	skybox->SetScale({2.5f, 2.5f, 2.5f});
    skybox->SetLocation(glm::vec3(0.0f, 0.0f, 0.0f));
    skybox->SetRotation(glm::quat(glm::vec3(0.0f, 3.14f, 0.0f)));
    skybox->SetModel("skybox");
    skybox->Init();
	
	Render::SetSunDirection(glm::normalize(glm::vec3(0.0f, 1.0f, 0.5f)));
	Render::SetSunColor(glm::vec3(250.0f, 214.0f, 165.0f) / 256.0f * 0.8f);
	Render::SetAmbientColor((glm::vec3(250.0f, 214.0f, 165.0f) / 256.0f * 0.8f) * 0.7f);

	LogicController* logic = new LogicController("dingus");
	
	WorldCell* centraltirgus = WorldCell::Make("demo4");
	centraltirgus->LoadFromDisk();
	centraltirgus->Load();

	player = new Player;
	//player->SetLocation(Entity::Find("player-start")->GetLocation());
	player->SetLocation(Entity::Find("bingus")->GetLocation() + vec3(0, 10, 0));
	player->Load();

	camera = new Ext::Camera::FirstPersonCamera;
	camera->SetMouselook(true);
	camera->SetRotateFollowing(true);
	camera->SetFollowingOffset({0.0f, 0.5f, 0.0f});
	camera->SetFollowing(player);
	
	camera->SetBobbingDistance(0.1f);
	camera->SetBobbingTilt(0.05f);
	camera->SetBobSpeed(0.2f);

	camera->SetBobbingCallback([](Ext::Camera::FirstPersonCamera*) {
		vec3 start = Render::GetViewPosition();
		vec3 direction = -DIRECTION_UP;
		vec3 forward = Render::GetViewRotation() * DIRECTION_FORWARD;
		vec3 side = Render::GetViewRotation() * DIRECTION_SIDE;
		
		start += 0.2f * side;
		
		auto result = Physics::Raycast(start, start + 2.0f * direction, ~(Physics::COLL_TRIGGER | Physics::COLL_PLAYER));
		
		if (result.collider) {
			auto res = Render::AABB::FindNearestFromRay(start, direction, -1);
			if (res.data && res.type == AABB::REFERENCE_RENDERCOMPONENT) {
				MaterialProperty prop = ((RenderComponent*)res.data)->GetModel()->GetMaterials()[res.triangle.material]->GetProperty();
				
				//AddLineMarker(0.3f * forward + result.point, COLOR_WHITE);
				
				name_t mat_name;
				if (prop == PROPERTY_WOOD) {
					mat_name = "wood";
				} else if (prop == PROPERTY_SLIME) {
					mat_name = "water";
				} else if (prop == PROPERTY_CONCRETE) {
					mat_name = "concrete";
				} else {
					mat_name = "metal";
				}
				
				SoundTable::Find(mat_name)->PlaySound(SOUND_WALK_ON, 0.3f * forward + result.point - 0.2f * side, 1.0f);
			}
		}
	});
	
	//Ext::Camera::SetCamera(camera);
	camera->SetActive();

	Model::Find("items/makaron")->AddReference();
	Model::Find("items/makaron")->Load();
	
	UI::BindKeyboardKey(UI::KEY_I, [](){
		UI::SetWindowSize(640/2, 480/2);
	});
	
	UI::BindKeyboardKey(UI::KEY_O, [](){
		TILT_WEIGHT = TILT_WEIGHT == 0.0f ? 1.0f : 0.0f;
	});
	
	UI::BindKeyboardKey(UI::KEY_P, [](){
		BOB_WEIGHT = BOB_WEIGHT == 0.0f ? 1.0f : 0.0f;
	});
	
	//UI::BindKeyboardKey(UI::KEY_LEFTMOUSE, [](){std::cout << "AAAA" << std::endl;});
	//UI::BindKeyboardKey(UI::KEY_LEFTMOUSE, KEY_ACTION_UP);
	
	Event::AddListener(Event::KEYPRESS, [](Event& event) {
        if (event.subtype != KEY_ACTION_ACTIVATE) return;
        
        vec3 start = Render::GetViewPosition();
        vec3 direction = Render::GetViewRotation() * DIRECTION_FORWARD;
        
        auto result = Physics::Raycast(start, start + 2.0f * direction, -1 ^ Physics::COLL_TRIGGER);
        
        if (result.collider) {
            Message::Send({Message::ACTIVATE, player->GetID(), result.collider->GetParent()->GetID(), 0});
        }
    });
    
    Event::AddListener(Event::KEYDOWN, [](Event& event) {
        if (event.subtype != KEY_ACTION_ACTIVATE) return;
        
        vec3 start = Render::GetViewPosition();
        vec3 direction = Render::GetViewRotation() * DIRECTION_FORWARD;
        
        auto result = Physics::Raycast(start, start + 2.0f * direction, -1 ^ Physics::COLL_TRIGGER);

		
		
        if (result.collider) {
			std::cout << result.collider << " " << result.collider->GetParent()->GetName() << std::endl;
            Message::Send({Message::ACTIVATE_ONCE, player->GetID(), result.collider->GetParent()->GetID(), 0});
        }
    });
	
	tram_follower_a = new PathFollower(Path::Find("tram-a"), Entity::Find("test-tram1")->GetLocation(), PATH_LINEAR);
	tram_follower_b = new PathFollower(Path::Find("tram-b"), Entity::Find("test-tram1")->GetLocation(), PATH_LINEAR);
	
	tram_follower_b->Advance(8.0f);
	
	//Physics::DRAW_PHYSICS_DEBUG = true;

	#ifdef __EMSCRIPTEN__
		UI::SetWebMainLoop(main_loop);
	#else
		while (!UI::ShouldExit()) {
			main_loop();
		}

		Async::Yeet();
		Audio::Uninit();
		UI::Uninit();
	#endif
}

#include <platform/api.h>
#include <cstring>

void DrawDialogAnswerBox(const char* text) {
	char textcpy[500];
	strcpy(textcpy, text);
	
	char* lines[10];
	int line_c = 0;
	
	lines[line_c++] = textcpy;
	
	for (char* c = textcpy; *c != '\0'; c++) {
		if (*c == '%' && *(c + 1) == 'n') {
			*c = '\0';
			c++;
			lines[line_c++] = c + 1;
		}
	}
	
	GUI::PushFrameRelative(GUI::FRAME_TOP, 10 + 16 * line_c);
	GUI::FillFrame(GUI::WIDGET_REVERSE_WINDOW);
	GUI::PushFrameRelative(GUI::FRAME_INSET, 5);

	for (int i = 0; i < line_c; i++) {
		GUI::Text(lines[i]);
		GUI::NewLine(GUI::LINE_LOW);
	}
	
	
	GUI::PopFrame();
	GUI::PopFrame();
}

bool language_selected = false;

void main_loop() {
	Core::Update();
	UI::Update();
	
	if (!language_selected) {
		if (UI::GetInputState() != UI::STATE_MENU_OPEN) {
			UI::SetInputState(UI::STATE_MENU_OPEN);
		}
		
		GUI::Begin();
		
		GUI::PushFrameRelative(GUI::FRAME_CENTER_HORIZONTAL, 200);
		GUI::PushFrameRelative(GUI::FRAME_CENTER_VERTICAL, 50);
		
			if (GUI::Button("Deutsch", true, 200)) {
				language_selected = true;
				UI::SetInputState(UI::STATE_DEFAULT);
				Language::Load("de");
			}
			GUI::NewLine();
		
			if (GUI::Button("English", true, 200)) {
				language_selected = true;
				UI::SetInputState(UI::STATE_DEFAULT);
				Language::Load("en");
			}
			GUI::NewLine();
			
			if (GUI::Button("Latviski", true, 200)) {
				language_selected = true;
				UI::SetInputState(UI::STATE_DEFAULT);
			}
		
		GUI::PopFrame();
		GUI::PopFrame();
	
		GUI::End();
	}
	
	
	double start = Platform::Window::GetTime();
	Physics::Update();
	double end = Platform::Window::GetTime();
	double delta  = 1000.0 * (end - start);
	//if (GetTick()%60 == 0)std::cout << "delta: " << delta << std::endl;
	
	
	// Camera tilting and bobbing control
	if (UI::PollKeyboardKey(UI::KEY_LEFT_CONTROL)) {
		camera->SetBobbingTilt(0.01f * TILT_WEIGHT);
		camera->SetBobSpeed(0.1f);
	} else if (UI::PollKeyboardKey(UI::KEY_LEFT_SHIFT)) {
		camera->SetBobbingTilt(0.05f * TILT_WEIGHT);
		camera->SetBobSpeed(0.195f);
	} else {
		camera->SetBobbingTilt(0.025f * TILT_WEIGHT);
		camera->SetBobSpeed(0.155f);
	}
	
	if (UI::PollKeyboardKey(UI::KEY_A)) {
		camera->SetTilt(0.1f * TILT_WEIGHT);
	} else if (UI::PollKeyboardKey(UI::KEY_D)) {
		camera->SetTilt(-0.1f * TILT_WEIGHT);
	} else {
		camera->SetTilt(0.0f * TILT_WEIGHT);
	}

	if (UI::PollKeyboardKey(UI::KEY_W) || UI::PollKeyboardKey(UI::KEY_S) ||
	UI::PollKeyboardKey(UI::KEY_A) || UI::PollKeyboardKey(UI::KEY_D)) {
		camera->SetBobbing(1.0f * BOB_WEIGHT);
	} else {
		camera->SetBobbing(0.0f * BOB_WEIGHT);
	}
	
	// UPDATE skybox
	skybox->SetLocation(Render::GetViewPosition());
	
	
	// FPS Counter
	static int last_fps = (int)GetTickTime();
	static int frames = 0;
	
	if (last_fps != (int)GetTickTime()) {
		last_fps = (int)GetTickTime();
		std::string title = (const char*)u8"Centrāltirgus Katakombas 3D | ";
		title += "Tramvaju rīkkopa v0.0.9 | ";
		title += std::to_string(frames);
		title += " fps";
		UI::SetWindowTitle(title.c_str());
		frames = 0;
	}
	
	frames++;
	
	

	
	
	
	GUI::Begin();
	Ext::Menu::Update();
	
	if (UI::GetInputState() != UI::STATE_MENU_OPEN) dialog = DIALOG_NONE;
	
	if (dialog != DIALOG_NONE) {
		GUI::PushFrameRelative(GUI::FRAME_CENTER_HORIZONTAL, 400);
		GUI::PushFrameRelative(GUI::FRAME_BOTTOM, 200);
		
		//GUI::PushFrameRelative(GUI::FRAME_TOP, 25);
		//GUI::FillFrame(GUI::WIDGET_REVERSE_WINDOW);
		//GUI::PushFrameRelative(GUI::FRAME_INSET, 5);
		/*switch (dialog) {
			case DIALOG_NONE:
	
			case BINGUS_HELLO:		GUI::Text(Language::Get("bingus-hello")); break;
			case BINGUS_WHATUP:		GUI::Text(Language::Get("bingus-a-whatup")); break;
			case BINUGS_WHERE_PEE:	GUI::Text(Language::Get("bingus-a-wherepee")); break;
			case BINUGS_PEE_OK:		GUI::Text(Language::Get("bingus-a-pook")); break;
			case BINUGS_HERE_MILK:	GUI::Text(Language::Get("bingus-a-heremilk")); break;
				
				
			case FLOPPA_HELLO:		GUI::Text(Language::Get("floppa-hello")); break;
			case FLOPPA_WHATUP:		GUI::Text(Language::Get("floppa-a-whatup")); break;
			case FLOPPA_WHATIS:		GUI::Text(Language::Get("floppa-a-whatis")); break;
			case FLOPPA_OKE:		GUI::Text(Language::Get("floppa-a-oke")); break;
			case FLOPPA_DIDIT:		GUI::Text(Language::Get("floppa-a-didit")); break;
			case FLOPPA_WASOKE:		GUI::Text(Language::Get("floppa-a-wasoke")); break;
			case FLOPPA_WASNOTOKE:	GUI::Text(Language::Get("floppa-a-wasnotoke")); break;
			case FLOPPA_WASMEH:		GUI::Text(Language::Get("floppa-a-wasmeh")); break;
			case FLOPPA_WHEREPEE:	GUI::Text(Language::Get("floppa-a-wherepee")); break;
			
			case BINGUS_GIVEMORE:	GUI::Text(Language::Get("bingus-a-givemore")); break;
			case BINGUS_DONEDONE:	GUI::Text(Language::Get("bingus-a-donedone")); break;
		}*/
		switch (dialog) {
			case DIALOG_NONE:
	
			case BINGUS_HELLO:		DrawDialogAnswerBox(Language::Get("bingus-hello")); break;
			case BINGUS_WHATUP:		DrawDialogAnswerBox(Language::Get("bingus-a-whatup")); break;
			case BINUGS_WHERE_PEE:	DrawDialogAnswerBox(Language::Get("bingus-a-wherepee")); break;
			case BINUGS_PEE_OK:		DrawDialogAnswerBox(Language::Get("bingus-a-pook")); break;
			case BINUGS_HERE_MILK:	DrawDialogAnswerBox(Language::Get("bingus-a-heremilk")); break;
				
				
			case FLOPPA_HELLO:		DrawDialogAnswerBox(Language::Get("floppa-hello")); break;
			case FLOPPA_WHATUP:		DrawDialogAnswerBox(Language::Get("floppa-a-whatup")); break;
			case FLOPPA_WHATIS:		DrawDialogAnswerBox(Language::Get("floppa-a-whatis")); break;
			case FLOPPA_OKE:		DrawDialogAnswerBox(Language::Get("floppa-a-oke")); break;
			case FLOPPA_DIDIT:		DrawDialogAnswerBox(Language::Get("floppa-a-didit")); break;
			case FLOPPA_WASOKE:		DrawDialogAnswerBox(Language::Get("floppa-a-wasoke")); break;
			case FLOPPA_WASNOTOKE:	DrawDialogAnswerBox(Language::Get("floppa-a-wasnotoke")); break;
			case FLOPPA_WASMEH:		DrawDialogAnswerBox(Language::Get("floppa-a-wasmeh")); break;
			case FLOPPA_WHEREPEE:	DrawDialogAnswerBox(Language::Get("floppa-a-wherepee")); break;
			
			case BINGUS_GIVEMORE:	DrawDialogAnswerBox(Language::Get("bingus-a-givemore")); break;
			case BINGUS_DONEDONE:	DrawDialogAnswerBox(Language::Get("bingus-a-donedone")); break;
		}
		
		//GUI::PopFrame();
		//GUI::PopFrame();

		GUI::NewLine(GUI::LINE_LOW);
		GUI::NewLine(GUI::LINE_LOW);
		GUI::NewLine(GUI::LINE_LOW);
		GUI::NewLine(GUI::LINE_LOW);
		//GUI::NewLine();
		

		switch (dialog) {
			case DIALOG_NONE:
			
			case BINGUS_GIVEMORE:
			case BINGUS_DONEDONE:
			
			case BINUGS_HERE_MILK:
			case BINUGS_PEE_OK:
			case BINGUS_WHATUP:
			case BINGUS_HELLO:
				if (!accept_milk_quest) {
					if (GUI::Button(Language::Get("bingus-q-whatup"))) {
						dialog = BINGUS_WHATUP;
						accept_milk_quest = true;
					}
					GUI::NewLine();
				}
				
				if (accept_milk_quest && collected_milk) {
					if (GUI::Button(Language::Get("bingus-q-heremilk"))) {
						dialog = BINUGS_HERE_MILK;
						collected_milk = 0;
						finish_milk_quest = true;
					}
					GUI::NewLine();
				}
				
				if (finish_milk_quest && !asked_for_more) {
					if (GUI::Button(Language::Get("bingus-q-givemore"))) {
						asked_for_more = true;
						dialog = BINGUS_GIVEMORE;
					}
					GUI::NewLine();
				}
				
				if (asked_for_more && floppa_finished) {
					if (GUI::Button(Language::Get("bingus-q-donedone"))) {
						dialog = BINGUS_DONEDONE;
					}
					GUI::NewLine();
				}
				
				if (GUI::Button(Language::Get("bingus-q-wherepee"))) {
					dialog = BINUGS_WHERE_PEE;
				}
				GUI::NewLine();
				
				if (GUI::Button(Language::Get("bingus-q-kbye"))) {
					dialog = DIALOG_NONE;
					UI::SetInputState(UI::STATE_DEFAULT);
				};
				
				break;
			
			case BINUGS_WHERE_PEE:
				if (GUI::Button(Language::Get("bingus-q-pook"))) {
					dialog = BINUGS_PEE_OK;
				}
				
				break;
				
		/*

floppa-hello Jā?
floppa-q-kbye Atā.
floppa-q-whatup Ko notiek?
floppa-a-whatup Es tikko tramvajam baterijas uzlādēju. Negribi izmest loku pa centrifugālo atpienojamo mezglu?
floppa-q-whatis 0_O Kas ir centrifugālais mezgls?
floppa-a-whatis Ā nu tu ieliec govi tramvajā un driftē ar viņu uz riņķi. Nu tā. Priekš piena kokteiļiem.
floppa-q-oke Labi. es  .. "izmetīšu" apli
floppa-a-oke Novēlu veiksmi .. haha .. >:)
floppa-q-didit Es izmetu. Apli.
floppa-a-didit WOW.. es tiešām nedomāju ka tu izdzīvosi ..   . . .. . kā bija?
floppa-q-wasoke Bija ok..
floppa-a-wasoke Vecīt...
floppa-q-wasnotoke Briesmīgi, es izkritu cauri pasaulei!
floppa-a-wasnotoke Izklausās sāpīgi...
floppa-q-wasmeh Eh..
floppa-a-wasmeh Eh........
floppa-q-wherepee Kur var pačurāt?
floppa-a-wherepee Uhh.. paprasi bingum.. es parasti čurāju izlietnē... piedod, bet es tev nevaru ļaut izmantot savējo izlietni..

*/		
				
			case FLOPPA_HELLO:
			case FLOPPA_WHATUP:
			case FLOPPA_WHATIS:
			case FLOPPA_OKE:
			case FLOPPA_WASOKE:
			case FLOPPA_WASNOTOKE:
			case FLOPPA_WASMEH:
			case FLOPPA_WHEREPEE:
			
			if (!floppa_whatupped) {
				if (GUI::Button(Language::Get("floppa-q-whatup"))) {
					dialog = FLOPPA_WHATUP;
					floppa_whatupped = true;
				}
				GUI::NewLine();
			} else {
				if (GUI::Button(Language::Get("floppa-q-whatis"))) {
					dialog = FLOPPA_WHATIS;
				}
				GUI::NewLine();
				
				if (!floppa_agreed && !floppa_finished) {
					if (GUI::Button(Language::Get("floppa-q-oke"))) {
						dialog = FLOPPA_OKE;
						floppa_agreed = true;
					}
					GUI::NewLine();
				}
				
				
				if (started_tramming && !floppa_finished) {
					if (GUI::Button(Language::Get("floppa-q-didit"))) {
						dialog = FLOPPA_DIDIT;
					}
					GUI::NewLine();
				}
			}
			
			if (GUI::Button(Language::Get("floppa-q-wherepee"))) {
				dialog = FLOPPA_WHEREPEE;
			}
			GUI::NewLine();
			
			if (GUI::Button(Language::Get("floppa-q-kbye"))) {
				dialog = DIALOG_NONE;
				UI::SetInputState(UI::STATE_DEFAULT);
			}
			
			break;
			
			case FLOPPA_DIDIT:
			if (GUI::Button(Language::Get("floppa-q-wasoke"))) {
				dialog = FLOPPA_WASOKE;
				floppa_finished = true;
			}
			GUI::NewLine();
			
			if (GUI::Button(Language::Get("floppa-q-wasnotoke"))) {
				dialog = FLOPPA_WASNOTOKE;
				floppa_finished = true;
			}
			GUI::NewLine();
			
			if (GUI::Button(Language::Get("floppa-q-wasmeh"))) {
				dialog = FLOPPA_WASMEH;
				floppa_finished = true;
			}
			
			break;			
		}
		
		
		
		GUI::PopFrame();
		GUI::PopFrame();
	}
	
	GUI::End();
	GUI::Update();
	
	
	//if (GetTickTime() > 5) {started_tramming = true; if(time_started_tramming < 0) {time_started_tramming = GetTickTime();}}
	
	if (started_tramming && !finished_tramming) {
		float speed = 8.0f;
		
		double time_since_started = GetTickTime() - time_started_tramming;
		
		if (time_since_started > 60.0) {
			speed *= (time_since_started / 15.0) - 3.0;
		}
		
		if (GetTick() % 60 == 0) std::cout << GetTickTime() - time_started_tramming << " speed " << speed <<  std::endl;
		
		//Render::AddLineMarker(tram_follower_a->GetPosition(), Render::COLOR_GREEN);
		//Render::AddLineMarker(tram_follower_b->GetPosition(), Render::COLOR_RED);
		
		//Path::Find("tram-a")->Draw();

		vec3 mid = glm::mix(tram_follower_a->GetPosition(), tram_follower_b->GetPosition(), 0.5f);
		vec3 dir = glm::normalize(tram_follower_b->GetPosition() - tram_follower_a->GetPosition());
		
		mid.y += 0.5f;
		
		Entity::Find("test-tram1")->SetLocation(mid);
		Entity::Find("test-tram1")->SetRotation(glm::quatLookAt(dir, DIRECTION_UP));
		
		tram_follower_a->Advance(GetDeltaTime() * speed);
		tram_follower_b->Advance(GetDeltaTime() * speed);
		
		if (glm::distance(tram_follower_a->GetPosition(), tram_follower_b->GetPosition()) > 12.0f) {
			tram_follower_a->Advance(GetDeltaTime());
		}
		
		if (time_since_started > 115.0) {
			finished_tramming = true;
		}
	}
	
	
	if (player->GetLocation().y < -15.0f) {
		player->SetLocation(Entity::Find("player-start")->GetLocation());
	}
	
	
#ifdef __EMSCRIPTEN__
	Async::LoadResourcesFromDisk();
#endif
	Async::LoadResourcesFromMemory();
	Async::FinishResources();
	
	Event::Dispatch();
	Message::Dispatch();
	
	//Entity::UpdateFromList();
	Entity::Update();

	//Loader::Update();

	if (Async::GetWaitingResources()) {
		std::cout << "Loading resources..." << std::endl;
	} else {
		ControllerComponent::Update();
	}
	
	Ext::Camera::Update();
	
	AnimationComponent::Update();
	
	//ControllerComponent::Update();
	
	Stats::Collate();
	
	Render::Render();
	UI::EndFrame();
}