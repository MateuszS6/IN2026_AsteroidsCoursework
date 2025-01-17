#include "Asteroid.h"
#include "Asteroids.h"
#include "Animation.h"
#include "AnimationManager.h"
#include "GameUtil.h"
#include "GameWindow.h"
#include "GameWorld.h"
#include "GameDisplay.h"
#include "Spaceship.h"
#include "BoundingSphere.h"
#include "GUILabel.h"
#include "Explosion.h"

// PUBLIC INSTANCE CONSTRUCTORS ///////////////////////////////////////////////

/** Constructor. Takes arguments from command line, just in case. */
Asteroids::Asteroids(int argc, char *argv[])
	: GameSession(argc, argv)
{
	mLevel = 0;
	mAsteroidCount = 0;
	mGameStarted = false;
	mSpaceshipSpriteName = "spaceship1";
	mIsBulletCooldownActive = false;
	mIsTimeSlowed = false;
}

/** Destructor. */
Asteroids::~Asteroids(void)
{
}

// PUBLIC INSTANCE METHODS ////////////////////////////////////////////////////

/** Start an Asteroids game. */
void Asteroids::Start()
{
	// Create a shared pointer for the Asteroids game object - DO NOT REMOVE
	shared_ptr<Asteroids> thisPtr = shared_ptr<Asteroids>(this);

	// Add this class as a listener of the game world
	mGameWorld->AddListener(thisPtr.get());

	// Add this as a listener to the world and the keyboard
	mGameWindow->AddKeyboardListener(thisPtr);

	// Add a score keeper to the game world
	mGameWorld->AddListener(&mScoreKeeper);

	// Add this class as a listener of the score keeper
	mScoreKeeper.AddListener(thisPtr);

	// Create an ambient light to show sprite textures
	GLfloat ambient_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat diffuse_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light);
	glEnable(GL_LIGHT0);

	// Animations for game objects
	Animation* explosion_anim = AnimationManager::GetInstance().CreateAnimationFromFile("explosion", 64, 1024, 64, 64, "explosion_fs.png");
	Animation* asteroid1_anim = AnimationManager::GetInstance().CreateAnimationFromFile("asteroid1", 128, 8192, 128, 128, "asteroid1_fs.png");
	Animation* spaceship1_anim = AnimationManager::GetInstance().CreateAnimationFromFile("spaceship1", 128, 128, 128, 128, "spaceship1_fs.png");
	Animation* spaceship2_anim = AnimationManager::GetInstance().CreateAnimationFromFile("spaceship2", 128, 128, 128, 128, "spaceship2_fs.png");
	Animation* spaceship3_anim = AnimationManager::GetInstance().CreateAnimationFromFile("spaceship3", 128, 128, 128, 128, "spaceship3_fs.png");
	Animation* spaceship4_anim = AnimationManager::GetInstance().CreateAnimationFromFile("spaceship4", 128, 128, 128, 128, "spaceship4_fs.png");
	Animation* spaceship5_anim = AnimationManager::GetInstance().CreateAnimationFromFile("spaceship5", 128, 128, 128, 128, "spaceship5_fs.png");
	Animation* powerup1_anim = AnimationManager::GetInstance().CreateAnimationFromFile("life", 128, 128, 128, 128, "life_bonus_fs.png");
	Animation* powerup2_anim = AnimationManager::GetInstance().CreateAnimationFromFile("time", 128, 128, 128, 128, "time_bonus_fs.png");
	Animation* powerup3_anim = AnimationManager::GetInstance().CreateAnimationFromFile("phasing", 128, 128, 128, 128, "phasing_bonus_fs.png");
	Animation* phasing_spaceship1_anim = AnimationManager::GetInstance().CreateAnimationFromFile("phasing_spaceship1", 128, 128, 128, 128, "phasing_spaceship1_fs.png");
	Animation* phasing_spaceship2_anim = AnimationManager::GetInstance().CreateAnimationFromFile("phasing_spaceship2", 128, 128, 128, 128, "phasing_spaceship2_fs.png");
	Animation* phasing_spaceship3_anim = AnimationManager::GetInstance().CreateAnimationFromFile("phasing_spaceship3", 128, 128, 128, 128, "phasing_spaceship3_fs.png");
	Animation* phasing_spaceship4_anim = AnimationManager::GetInstance().CreateAnimationFromFile("phasing_spaceship4", 128, 128, 128, 128, "phasing_spaceship4_fs.png");
	Animation* phasing_spaceship5_anim = AnimationManager::GetInstance().CreateAnimationFromFile("phasing_spaceship5", 128, 128, 128, 128, "phasing_spaceship5_fs.png");

	// Create the GUI
	CreateGUI();

	// Create a default spaceship and add it to the world
	mGameWorld->AddObject(CreateSpaceship());

	// Add a player (watcher) to the game world
	mGameWorld->AddListener(&mPlayer);

	// Add this class as a listener of the player
	mPlayer.AddListener(thisPtr);

	// Start the game
	GameSession::Start();
}

/** Stop the current game. */
void Asteroids::Stop()
{
	// Stop the game
	GameSession::Stop();
}

// PUBLIC INSTANCE METHODS IMPLEMENTING IKeyboardListener /////////////////////

void Asteroids::OnKeyPressed(uchar key, int x, int y)
{
	if (!mGameStarted) {
		if (key == ' ') {
			// Hide the start screen labels
			mTitleLabel->SetVisible(false);
			mStartLabel->SetVisible(false);
			mSelectLabel->SetVisible(false);
			mControlsLabel->SetVisible(false);
			// Show the in-game labels
			mScoreLabel->SetVisible(true);
			mLivesLabel->SetVisible(true);
			// Set the game to 'started' so that controls react correctly
			mGameStarted = true;
			// Create some asteroids and add them to the world
			CreateAsteroids(10);
			// Create a power-up after 5 seconds
			SetTimer(5000, SPAWN_NEXT_POWERUP);
		}
		else if (key >= '1' && key <= '5') {
			mSpaceshipSpriteName = "spaceship" + to_string(key - '0');
			SetSpaceshipSprite(mSpaceshipSpriteName);
		}
	}
	else {
		if (key == '1') mSpaceship->Shoot(false);
		else if (key == '2' && !mIsBulletCooldownActive) {
			mSpaceship->Shoot(true);
			ToggleBulletCooldown(true);
		}
	}
}

void Asteroids::OnKeyReleased(uchar key, int x, int y) {}

void Asteroids::OnSpecialKeyPressed(int key, int x, int y)
{
	switch (key)
	{
	// If up arrow key is pressed AND the game has started, start applying forward thrust
	case GLUT_KEY_UP: if (mGameStarted) mSpaceship->Thrust(6); break;
	// If left arrow key is pressed start rotating anti-clockwise
	case GLUT_KEY_LEFT: mSpaceship->Rotate(150); break;
	// If right arrow key is pressed start rotating clockwise
	case GLUT_KEY_RIGHT: mSpaceship->Rotate(-150); break;
	// Default case - do nothing
	default: break;
	}
}

void Asteroids::OnSpecialKeyReleased(int key, int x, int y)
{
	switch (key)
	{
	// If up arrow key is released stop applying forward thrust
	case GLUT_KEY_UP: mSpaceship->Thrust(0); break;
	// If left arrow key is released stop rotating
	case GLUT_KEY_LEFT: mSpaceship->Rotate(0); break;
	// If right arrow key is released stop rotating
	case GLUT_KEY_RIGHT: mSpaceship->Rotate(0); break;
	// Default case - do nothing
	default: break;
	}
}


// PUBLIC INSTANCE METHODS IMPLEMENTING IGameWorldListener ////////////////////

void Asteroids::OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object)
{
	// Handle asteroid removal
	if (object->GetType() == GameObjectType("Asteroid"))
	{
		// Create and position the explosion
		shared_ptr<GameObject> explosion = CreateExplosion();
		explosion->SetPosition(object->GetPosition());
		explosion->SetRotation(object->GetRotation());
		mGameWorld->AddObject(explosion);

		// Efficiently remove the asteroid from the list
		mAsteroidList.erase(remove(mAsteroidList.begin(), mAsteroidList.end(), object), mAsteroidList.end());

		// Decrease asteroid count and possibly start next level
		if (--mAsteroidCount <= 0) SetTimer(500, START_NEXT_LEVEL); 
		return; // Done handling asteroid removal
	}

	// Handle power-up removal
	if (object->GetType() == GameObjectType("PowerUp"))
	{
		SetTimer(10000, SPAWN_NEXT_POWERUP);
		return; // Done handling power-up removal
	}
}

// PUBLIC INSTANCE METHODS IMPLEMENTING ITimerListener ////////////////////////

void Asteroids::OnTimer(int value)
{
	switch (value)
	{
	case CREATE_NEW_PLAYER:
		mSpaceship->Reset();
		mGameWorld->AddObject(mSpaceship);
		break;

	case START_NEXT_LEVEL:
	{
		// Enclose in braces to create a local scope
		mLevel++;
		int num_asteroids = 10 + 2 * mLevel;
		CreateAsteroids(num_asteroids);
		if (mIsTimeSlowed) ToggleSlowMotion(true);
		break;
	}

	case SHOW_GAME_OVER:
		mGameOverLabel->SetVisible(true);
		break;

	case SPAWN_NEXT_POWERUP:
		CreatePowerUp();
		break;

	case END_SLOW_MOTION:
		ToggleSlowMotion(false);
		break;

	case END_PHASING:
		TogglePhasing(false);
		break;

	case END_BULLET_COOLDOWN:
		ToggleBulletCooldown(false);

	default:
		// Handle unexpected timer value
		break;
	}
}

// PROTECTED INSTANCE METHODS /////////////////////////////////////////////////

shared_ptr<GameObject> Asteroids::CreateSpaceship()
{
	// Create a raw pointer to a spaceship that can be converted to
	// shared_ptrs of different types because GameWorld implements IRefCount
	mSpaceship = make_shared<Spaceship>();
	TogglePhasing(false);
	shared_ptr<Shape> bullet_shape_1 = make_shared<Shape>("bullet1.shape");
	mSpaceship->SetPrimaryBulletShape(bullet_shape_1);
	shared_ptr<Shape> bullet_shape_2 = make_shared<Shape>("bullet2.shape");
	mSpaceship->SetSecondaryBulletShape(bullet_shape_2);
	mSpaceship->SetScale(0.1f);
	// Reset spaceship back to centre of the world
	mSpaceship->Reset();
	// Return the spaceship so it can be added to the world
	return mSpaceship;
}

void Asteroids::SetSpaceshipSprite(const string& name)
{
	Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName(name);
	shared_ptr<Sprite> spaceship_sprite = make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	mSpaceship->SetSprite(spaceship_sprite);
}

void Asteroids::CreateAsteroids(const uint num_asteroids)
{
	mAsteroidCount = num_asteroids;
	for (uint i = 0; i < num_asteroids; i++)
	{
		Animation *anim_ptr = AnimationManager::GetInstance().GetAnimationByName("asteroid1");
		shared_ptr<Sprite> asteroid_sprite = make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
		asteroid_sprite->SetLoopAnimation(true);
		shared_ptr<GameObject> asteroid = make_shared<Asteroid>();
		asteroid->SetBoundingShape(make_shared<BoundingSphere>(asteroid->GetThisPtr(), 10.0f));
		asteroid->SetSprite(asteroid_sprite);
		asteroid->SetScale(0.2f);
		mGameWorld->AddObject(asteroid);
		mAsteroidList.push_back(dynamic_pointer_cast<Asteroid>(asteroid));
	}
}

void Asteroids::CreatePowerUp()
{
	// Define an array of power-up names
	const vector<string> powerUpNames = { "life", "time", "phasing" };

	// Generate a random index based on the number of available power-ups
	int num = rand() % powerUpNames.size();

	// Access the power-up name using the random index
	const string& name = powerUpNames[num];

	// Create the power-up object
	auto powerup = make_shared<PowerUp>(name);
	powerup->SetBoundingShape(make_shared<BoundingSphere>(powerup->GetThisPtr(), 5.0f));

	// Retrieve animation based on power-up name
	auto anim_ptr = AnimationManager::GetInstance().GetAnimationByName(name);
	auto powerup_sprite = make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);

	// Set the sprite and scale
	powerup->SetSprite(powerup_sprite);
	powerup->SetScale(0.1f);

	// Add the power-up to the game world
	mGameWorld->AddObject(powerup);
}

void Asteroids::ToggleSlowMotion(bool activate)
{
	// Update the state variable for slow motion
	mIsTimeSlowed = activate;
	// Iterate over all asteroids and adjust their speed based on the activation status
	for (auto& asteroid : mAsteroidList)
	{
		if (activate) asteroid->SlowDown(); // Reduce speed to create a slow-motion effect
		else asteroid->ResetSpeed(); // Restore original speed when not in slow motion
	}
	// Update the power-up label's text and color to reflect the current state of slow motion
	UpdatePowerUpLabel(activate ? "TIME SLOWED" : "", GLVector3f(0.0f, 0.6f, 0.9f));
}

void Asteroids::TogglePhasing(bool activate)
{
	// Set the spaceship's bounding shape to null (no collisions) when phasing is active
	mSpaceship->SetBoundingShape(activate ? nullptr : make_shared<BoundingSphere>(mSpaceship->GetThisPtr(), 4.0f));
	// Update the spaceship sprite to indicate whether it is in phasing mode or not
	SetSpaceshipSprite(activate ? "phasing_" + mSpaceshipSpriteName : mSpaceshipSpriteName);
	// Update the power-up label's text and color based on the phasing status
	UpdatePowerUpLabel(activate ? "PHASING" : "", GLVector3f(0.7f, 0.5f, 0.3f));
}

void Asteroids::ToggleBulletCooldown(bool activate)
{
	// Update the state variable for bullet cooldown management
	mIsBulletCooldownActive = activate;
	// Set a timer to end the cooldown after 1500 milliseconds if activated
	if (activate) SetTimer(1500, END_BULLET_COOLDOWN);
}

void Asteroids::CreateGUI()
{
	// Add a (transparent) border around the edge of the game display
	mGameDisplay->GetContainer()->SetBorder(GLVector2i(10, 10));

	// Create the start screen elements
	CreateStartScreen();

	// Create a new GUILabel and wrap it up in a shared_ptr
	mScoreLabel = make_shared<GUILabel>("Score: 0");
	// Set the vertical alignment of the label to GUI_VALIGN_TOP
	mScoreLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	// Set visiblity to false (hidden)
	mScoreLabel->SetVisible(false);
	// Add the GUILabel to the GUIComponent  
	shared_ptr<GUIComponent> score_component = static_pointer_cast<GUIComponent>(mScoreLabel);
	mGameDisplay->GetContainer()->AddComponent(score_component, GLVector2f(0.0f, 1.0f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mPowerUpLabel = make_shared<GUILabel>("");
	mPowerUpLabel->SetSize(30);
	// Set the vertical alignment of the label to GUI_VALIGN_TOP
	mPowerUpLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	// Add the GUILabel to the GUIComponent  
	shared_ptr<GUIComponent> bonus_component = static_pointer_cast<GUIComponent>(mPowerUpLabel);
	mGameDisplay->GetContainer()->AddComponent(bonus_component, GLVector2f(0.0f, 0.1f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mLivesLabel = make_shared<GUILabel>("Lives: 3");
	// Set the vertical alignment of the label to GUI_VALIGN_BOTTOM
	mLivesLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	// Set the visiblity to false (hidden)
	mLivesLabel->SetVisible(false);
	// Add the GUILabel to the GUIComponent  
	shared_ptr<GUIComponent> lives_component = static_pointer_cast<GUIComponent>(mLivesLabel);
	mGameDisplay->GetContainer()->AddComponent(lives_component, GLVector2f(0.0f, 0.0f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mGameOverLabel = shared_ptr<GUILabel>(new GUILabel("GAME OVER"));
	// Set the horizontal alignment of the label to GUI_HALIGN_CENTER
	mGameOverLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	// Set the vertical alignment of the label to GUI_VALIGN_MIDDLE
	mGameOverLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	// Set the visibility to false (hidden)
	mGameOverLabel->SetVisible(false);
	// Add the GUILabel to the GUIContainer  
	shared_ptr<GUIComponent> game_over_component = static_pointer_cast<GUIComponent>(mGameOverLabel);
	mGameDisplay->GetContainer()->AddComponent(game_over_component, GLVector2f(0.5f, 0.5f));
}

void Asteroids::CreateStartScreen()
{
	// Create and wrap the title label in a shared_ptr
	mTitleLabel = shared_ptr<GUILabel>(new GUILabel("Asteroids"));
	// Align to the middle of the screen
	mTitleLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	// Align to the centre of the screen
	mTitleLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	// Add to the display and set distance from centre/middle
	shared_ptr<GUIComponent> title_component = static_pointer_cast<GUIComponent>(mTitleLabel);
	mGameDisplay->GetContainer()->AddComponent(title_component, GLVector2f(0.5f, 0.7f));

	// Create and wrap the start label in a shared_ptr
	mStartLabel = shared_ptr<GUILabel>(new GUILabel("Press [SPACE] to Start"));
	// Align to the middle of the screen
	mStartLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	// Align to the centre of the screen
	mStartLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	// Add to the display and set distance from centre/middle
	shared_ptr<GUIComponent> start_component = static_pointer_cast<GUIComponent>(mStartLabel);
	mGameDisplay->GetContainer()->AddComponent(start_component, GLVector2f(0.5f, 0.3f));

	// Create and wrap the select label in a shared_ptr
	mSelectLabel = shared_ptr<GUILabel>(new GUILabel("Press [1-5] to Select Spaceship"));
	// Align to the bottom of the screen
	mSelectLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	// Align to the centre of the screen
	mSelectLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	// Add to the display and set distance from centre/middle
	shared_ptr<GUIComponent> select_component = static_pointer_cast<GUIComponent>(mSelectLabel);
	mGameDisplay->GetContainer()->AddComponent(select_component, GLVector2f(0.5f, 0.2f));

	// Create and wrap the controls label in a shared_ptr
	mControlsLabel = shared_ptr<GUILabel>(new GUILabel("[1/2] for Primary/Secondary Fire"));
	// Align to the bottom of the screen
	mControlsLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	// Align to the centre of the screen
	mControlsLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	// Add to the display and set distance from centre/middle
	shared_ptr<GUIComponent> controls_component = static_pointer_cast<GUIComponent>(mControlsLabel);
	mGameDisplay->GetContainer()->AddComponent(controls_component, GLVector2f(0.5f, 0.1f));
}

void Asteroids::UpdateLivesLabel()
{
	// Format the lives left message using a string-based stream
	ostringstream msg_stream;
	msg_stream << "Lives: " << mPlayer.GetLivesLeft();
	// Get the lives left message as a string
	string lives_msg = msg_stream.str();
	mLivesLabel->SetText(lives_msg);
}

void Asteroids::UpdatePowerUpLabel(const string& text, const GLVector3f& colour)
{
	mPowerUpLabel->SetText(text);
	if (!text.empty()) mPowerUpLabel->SetColor(colour);
}

void Asteroids::OnScoreChanged(int score)
{
	// Format the score message using an string-based stream
	ostringstream msg_stream;
	msg_stream << "Score: " << score;
	// Get the score message as a string
	string score_msg = msg_stream.str();
	mScoreLabel->SetText(score_msg);
}

void Asteroids::OnPlayerKilled()
{
	shared_ptr<GameObject> explosion = CreateExplosion();
	explosion->SetPosition(mSpaceship->GetPosition());
	explosion->SetRotation(mSpaceship->GetRotation());
	mGameWorld->AddObject(explosion);

	UpdateLivesLabel();

	if (mPlayer.GetLivesLeft() > 0) SetTimer(1000, CREATE_NEW_PLAYER);
	else SetTimer(500, SHOW_GAME_OVER);
}

void Asteroids::OnPowerUpCollected(const string& powerUpType)
{
	if (powerUpType == "life")
	{
		mPlayer.AddLife();
		UpdateLivesLabel();
	}
	else if (powerUpType == "time")
	{
		ToggleSlowMotion(true);
		SetTimer(10000, END_SLOW_MOTION);
	}
	else if (powerUpType == "phasing")
	{
		TogglePhasing(true);
		SetTimer(10000, END_PHASING);
	}
}

shared_ptr<GameObject> Asteroids::CreateExplosion()
{
	Animation *anim_ptr = AnimationManager::GetInstance().GetAnimationByName("explosion");
	shared_ptr<Sprite> explosion_sprite = make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	explosion_sprite->SetLoopAnimation(false);
	shared_ptr<GameObject> explosion = make_shared<Explosion>();
	explosion->SetSprite(explosion_sprite);
	explosion->Reset();
	return explosion;
}