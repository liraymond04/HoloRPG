#ifndef MAINMENU_H
#define MAINMENU_H

#include "GameState.h"

class Scene;

class MainMenu : public GameState {
protected:
    MainMenu() {}

private:
    static MainMenu m_MainMenu;

    Game* game;
    vector<Scene*> scenes;

    string sStateName = "Main Menu";

public:
    /* Core */
    void Init(Game* g);
    void Cleanup();

    /* Scene */
    void ChangeScene(Scene* scene);
    void PushScene(Scene* scene);
    void PopScene();

    /* Flow */
    void Pause();
    void Resume();

    /* Loop */
    bool HandleEvents();
    bool Update();
    bool Draw();

    static MainMenu* Instance() {
		return &m_MainMenu;
	}
};

#endif