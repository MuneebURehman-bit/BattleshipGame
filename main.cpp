#include <SFML/Graphics.hpp>
#include <cmath>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

using namespace sf;
using namespace std;

const int BOARD_N = 10;
const int TILE = 38;
const int LEFT_BOARD_X = 60;
const int RIGHT_BOARD_X = 560;
const int BOARD_TOP = 115;

const int SCR_SETUP = 1;
const int SCR_FIGHT = 3;

const int SHIP_COUNT = 5;
vector<int> shipLengths = { 5, 4, 3, 3, 2 };

int p1Ships[BOARD_N][BOARD_N];
int p2Ships[BOARD_N][BOARD_N];
int p1Shots[BOARD_N][BOARD_N];
int p2Shots[BOARD_N][BOARD_N];

vector<int> p1Health(SHIP_COUNT, 0);
vector<int> p2Health(SHIP_COUNT, 0);
vector<int> p1Row(SHIP_COUNT, -1);
vector<int> p2Row(SHIP_COUNT, -1);
vector<int> p1Col(SHIP_COUNT, -1);
vector<int> p2Col(SHIP_COUNT, -1);
vector<int> p1Dir(SHIP_COUNT, 0);
vector<int> p2Dir(SHIP_COUNT, 0);

int currentScreen = SCR_SETUP;
int activePlayer = 0;
int placingStep = 0;
int placeDir = 0;

bool singlePlayer = true;

float curMouseX = 0.0f;
float curMouseY = 0.0f;

RenderWindow app;
Font mainFont;
Clock totalClock;

int getShipCell(int p, int r, int c)
{
    if (p == 0)
    {
        return p1Ships[r][c];
    }
    return p2Ships[r][c];
}

void setShipCell(int p, int r, int c, int val)
{
    if (p == 0)
    {
        p1Ships[r][c] = val;
    }
    else
    {
        p2Ships[r][c] = val;
    }
}

int getShotCell(int p, int r, int c)
{
    if (p == 0)
    {
        return p1Shots[r][c];
    }
    return p2Shots[r][c];
}

int getHealth(int p, int idx)
{
    if (p == 0)
    {
        return p1Health[idx];
    }
    return p2Health[idx];
}

int getShipRow(int p, int idx)
{
    if (p == 0)
    {
        return p1Row[idx];
    }
    return p2Row[idx];
}

int getShipCol(int p, int idx)
{
    if (p == 0)
    {
        return p1Col[idx];
    }
    return p2Col[idx];
}

int getShipDir(int p, int idx)
{
    if (p == 0)
    {
        return p1Dir[idx];
    }
    return p2Dir[idx];
}

void saveShipMeta(int p, int idx, int r, int c, int dir)
{
    if (p == 0)
    {
        p1Row[idx] = r;
        p1Col[idx] = c;
        p1Dir[idx] = dir;
    }
    else
    {
        p2Row[idx] = r;
        p2Col[idx] = c;
        p2Dir[idx] = dir;
    }
}

void clearMatchData()
{
    for (int r = 0; r < BOARD_N; r++)
    {
        for (int c = 0; c < BOARD_N; c++)
        {
            p1Ships[r][c] = 0;
            p2Ships[r][c] = 0;
            p1Shots[r][c] = 0;
            p2Shots[r][c] = 0;
        }
    }

    for (int i = 0; i < SHIP_COUNT; i++)
    {
        p1Health[i] = shipLengths[i];
        p2Health[i] = shipLengths[i];
        p1Row[i] = -1;
        p2Row[i] = -1;
        p1Col[i] = -1;
        p2Col[i] = -1;
        p1Dir[i] = 0;
        p2Dir[i] = 0;
    }

    activePlayer = 0;
    placingStep = 0;
    placeDir = 0;
}

string getName(int p)
{
    if (singlePlayer && p == 1)
    {
        return "Computer";
    }
    if (p == 0)
    {
        return "Player 1";
    }
    return "Player 2";
}

int getViewerId()
{
    if (singlePlayer)
    {
        return 0;
    }
    return activePlayer;
}

int countAliveShips(int p)
{
    int alive = 0;
    for (int i = 0; i < SHIP_COUNT; i++)
    {
        if (getHealth(p, i) > 0)
        {
            alive++;
        }
    }
    return alive;
}

bool canPlaceHere(int p, int startR, int startC, int len, int dir)
{
    if (startR < 0 || startC < 0)
    {
        return false;
    }

    if (dir == 1)
    {
        if (startR + len > BOARD_N)
        {
            return false;
        }
    }
    else
    {
        if (startC + len > BOARD_N)
        {
            return false;
        }
    }

    for (int k = 0; k < len; k++)
    {
        int r = startR;
        int c = startC;
        if (dir == 1)
        {
            r = startR + k;
        }
        else
        {
            c = startC + k;
        }

        if (getShipCell(p, r, c) != 0)
        {
            return false;
        }
    }

    return true;
}

void writeShipToGrid(int p, int id, int startR, int startC, int dir)
{
    saveShipMeta(p, id, startR, startC, dir);
    int len = shipLengths[id];

    for (int k = 0; k < len; k++)
    {
        if (dir == 1)
        {
            setShipCell(p, startR + k, startC, id + 1);
        }
        else
        {
            setShipCell(p, startR, startC + k, id + 1);
        }
    }
}

void drawStr(string msg, float x, float y, int sz, Color col)
{
    Text label(mainFont, msg, sz);
    label.setPosition({ x, y });
    label.setFillColor(col);
    app.draw(label);
}

void drawStrCenter(string msg, float y, int sz, Color col)
{
    Text label(mainFont, msg, sz);
    FloatRect box = label.getLocalBounds();
    float x = 500.0f - (box.position.x + box.size.x * 0.5f);
    label.setPosition({ x, y });
    label.setFillColor(col);
    app.draw(label);
}

void fillBox(float x, float y, float w, float h, Color col)
{
    RectangleShape r({ w, h });
    r.setPosition({ x, y });
    r.setFillColor(col);
    app.draw(r);
}

bool mouseToGrid(int bx, int by, int& outR, int& outC)
{
    int c = (int)floor((curMouseX - (float)bx) / (float)TILE);
    int r = (int)floor((curMouseY - (float)by) / (float)TILE);

    if (r < 0 || r >= BOARD_N || c < 0 || c >= BOARD_N)
    {
        return false;
    }

    outR = r;
    outC = c;
    return true;
}

void drawVessel(float x, float y, int len, int dir, Color bodyCol)
{
    float span = (float)(len * TILE);
    Color trim(bodyCol.r * 3 / 4, bodyCol.g * 3 / 4, bodyCol.b * 3 / 4);

    if (dir == 0)
    {
        ConvexShape hull(5);
        hull.setPoint(0, Vector2f(x + 4.0f, y + 6.0f));
        hull.setPoint(1, Vector2f(x + span - 18.0f, y + 6.0f));
        hull.setPoint(2, Vector2f(x + span - 3.0f, y + TILE * 0.5f));
        hull.setPoint(3, Vector2f(x + span - 18.0f, y + TILE - 6.0f));
        hull.setPoint(4, Vector2f(x + 4.0f, y + TILE - 6.0f));
        hull.setFillColor(bodyCol);
        hull.setOutlineColor(Color(20, 24, 34));
        hull.setOutlineThickness(2.0f);
        app.draw(hull);

        fillBox(x + 12.0f, y + 11.0f, span - 36.0f, TILE - 22.0f, trim);
        fillBox(x + span * 0.36f, y + 9.0f, 18.0f, TILE - 18.0f, Color(215, 220, 230));

        CircleShape turret(4.0f);
        turret.setOrigin({ 4.0f, 4.0f });
        turret.setPosition({ x + span * 0.36f + 9.0f, y + TILE * 0.5f });
        turret.setFillColor(Color(55, 60, 70));
        app.draw(turret);
    }
    else
    {
        ConvexShape hull(5);
        hull.setPoint(0, Vector2f(x + 6.0f, y + 4.0f));
        hull.setPoint(1, Vector2f(x + TILE - 6.0f, y + 4.0f));
        hull.setPoint(2, Vector2f(x + TILE - 6.0f, y + span - 18.0f));
        hull.setPoint(3, Vector2f(x + TILE * 0.5f, y + span - 3.0f));
        hull.setPoint(4, Vector2f(x + 6.0f, y + span - 18.0f));
        hull.setFillColor(bodyCol);
        hull.setOutlineColor(Color(20, 24, 34));
        hull.setOutlineThickness(2.0f);
        app.draw(hull);

        fillBox(x + 11.0f, y + 12.0f, TILE - 22.0f, span - 36.0f, trim);
        fillBox(x + 9.0f, y + span * 0.36f, TILE - 18.0f, 18.0f, Color(215, 220, 230));

        CircleShape turret(4.0f);
        turret.setOrigin({ 4.0f, 4.0f });
        turret.setPosition({ x + TILE * 0.5f, y + span * 0.36f + 9.0f });
        turret.setFillColor(Color(55, 60, 70));
        app.draw(turret);
    }
}

void drawBoard(int p, int bx, int by, bool revealAll, float t)
{
    fillBox((float)bx - 4.0f, (float)by - 4.0f,
        (float)(BOARD_N * TILE) + 8.0f, (float)(BOARD_N * TILE) + 8.0f,
        Color(10, 20, 35, 210));

    for (int r = 0; r < BOARD_N; r++)
    {
        for (int c = 0; c < BOARD_N; c++)
        {
            int waveG = (int)(72.0f + 20.0f * sin(t * 2.1f + (float)r * 0.65f + (float)c * 0.65f));
            fillBox((float)(bx + c * TILE + 1), (float)(by + r * TILE + 1),
                (float)(TILE - 2), (float)(TILE - 2), Color(18, waveG, 148));
        }
    }

    for (int i = 0; i < SHIP_COUNT; i++)
    {
        int sr = getShipRow(p, i);
        int sc = getShipCol(p, i);
        if (sr < 0 || sc < 0)
        {
            continue;
        }

        bool destroyed = (getHealth(p, i) == 0);
        if (revealAll || destroyed)
        {
            Color col(128, 138, 156);
            if (destroyed)
            {
                col = Color(155, 60, 60);
            }
            drawVessel((float)(bx + sc * TILE), (float)(by + sr * TILE),
                shipLengths[i], getShipDir(p, i), col);
        }
    }

    for (int r = 0; r < BOARD_N; r++)
    {
        for (int c = 0; c < BOARD_N; c++)
        {
            int mark = getShotCell(p, r, c);
            if (mark == 0)
            {
                continue;
            }

            float cx = (float)bx + ((float)c + 0.5f) * (float)TILE;
            float cy = (float)by + ((float)r + 0.5f) * (float)TILE;

            if (mark == 1)
            {
                CircleShape dot(5.0f);
                dot.setOrigin({ 5.0f, 5.0f });
                dot.setPosition({ cx, cy });
                dot.setFillColor(Color(235, 245, 255));
                app.draw(dot);
            }
            else if (mark == 2)
            {
                CircleShape hit(12.0f);
                hit.setOrigin({ 12.0f, 12.0f });
                hit.setPosition({ cx, cy });
                hit.setFillColor(Color(255, 70, 20, 210));
                app.draw(hit);
            }
        }
    }
}

void drawGhostPreview(int bx, int by, int p, int len, int dir)
{
    int r = 0;
    int c = 0;
    if (!mouseToGrid(bx, by, r, c))
    {
        return;
    }

    bool valid = canPlaceHere(p, r, c, len, dir);
    Color tint(240, 45, 45, 130);
    if (valid)
    {
        tint = Color(45, 235, 85, 130);
    }

    for (int k = 0; k < len; k++)
    {
        int rr = r;
        int cc = c;
        if (dir == 1)
        {
            rr = r + k;
        }
        else
        {
            cc = c + k;
        }

        if (rr < BOARD_N && cc < BOARD_N)
        {
            fillBox((float)(bx + cc * TILE + 1), (float)(by + rr * TILE + 1),
                (float)(TILE - 2), (float)(TILE - 2), tint);
        }
    }
}

void renderSetup(float t)
{
    string header = getName(activePlayer) + " - Ship " + to_string(placingStep + 1)
        + " of 5 (Length " + to_string(shipLengths[placingStep])
        + ")   [R / Right Click: Rotate]";
    drawStr(header, 60.0f, 28.0f, 23, Color::White);

    if (singlePlayer)
    {
        drawStr("Place your fleet on the left grid. Computer ships stay hidden.",
            60.0f, 64.0f, 19, Color(255, 215, 70));
    }
    else
    {
        drawStr("Other player: look away while ships are being placed.",
            60.0f, 64.0f, 19, Color(255, 215, 70));
    }

    drawBoard(activePlayer, LEFT_BOARD_X, BOARD_TOP, true, t);
    drawGhostPreview(LEFT_BOARD_X, BOARD_TOP, activePlayer, shipLengths[placingStep], placeDir);
}

void renderBattle(float t)
{
    int v = getViewerId();
    string topMsg = getName(activePlayer) + "'s turn";

    drawStrCenter(topMsg, 18.0f, 34, Color(255, 215, 60));

    drawStr("Your Fleet (Alive: " + to_string(countAliveShips(v)) + ")",
        (float)LEFT_BOARD_X, 78.0f, 21, Color::White);
    drawStr("Enemy Waters (Alive: " + to_string(countAliveShips(1 - v)) + ")",
        (float)RIGHT_BOARD_X, 78.0f, 21, Color::White);

    drawBoard(v, LEFT_BOARD_X, BOARD_TOP, true, t);
    drawBoard(1 - v, RIGHT_BOARD_X, BOARD_TOP, false, t);

    int hr = 0;
    int hc = 0;
    if (currentScreen == SCR_FIGHT && (activePlayer == 0 || !singlePlayer))
    {
        if (mouseToGrid(RIGHT_BOARD_X, BOARD_TOP, hr, hc))
        {
            RectangleShape cursor({ (float)(TILE - 2), (float)(TILE - 2) });
            cursor.setPosition({
                (float)(RIGHT_BOARD_X + hc * TILE + 1),
                (float)(BOARD_TOP + hr * TILE + 1)
            });
            cursor.setFillColor(Color::Transparent);
            cursor.setOutlineColor(Color::Yellow);
            cursor.setOutlineThickness(2.0f);
            app.draw(cursor);
        }
    }
}

void clickSetup(bool leftBtn)
{
    if (!leftBtn)
    {
        placeDir = 1 - placeDir;
        return;
    }

    int r = 0;
    int c = 0;
    if (mouseToGrid(LEFT_BOARD_X, BOARD_TOP, r, c))
    {
        if (canPlaceHere(activePlayer, r, c, shipLengths[placingStep], placeDir))
        {
            writeShipToGrid(activePlayer, placingStep, r, c, placeDir);
            placingStep++;

            if (placingStep == SHIP_COUNT)
            {
                currentScreen = SCR_FIGHT;
            }
        }
    }
}

void handleMouseClick(bool leftBtn)
{
    switch (currentScreen)
    {
    case SCR_SETUP:
        clickSetup(leftBtn);
        break;
    }
}

void handleKeyPress(Keyboard::Key key)
{
    if (key == Keyboard::Key::R)
    {
        if (currentScreen == SCR_SETUP)
        {
            placeDir = 1 - placeDir;
        }
        return;
    }
}

bool loadSystemFont()
{
    vector<string> paths = {
        "arial.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf"
    };

    for (int i = 0; i < (int)paths.size(); i++)
    {
        if (filesystem::exists(paths[i]) && mainFont.openFromFile(paths[i]))
        {
            return true;
        }
    }
    return false;
}

int main()
{
    app.create(VideoMode({ 1000, 600 }), "Battleships", Style::Titlebar | Style::Close);
    app.setFramerateLimit(60);

    if (!loadSystemFont())
    {
        return 1;
    }

    clearMatchData();

    while (app.isOpen())
    {
        float elapsed = totalClock.getElapsedTime().asSeconds();

        Vector2i mpos = Mouse::getPosition(app);
        curMouseX = (float)mpos.x;
        curMouseY = (float)mpos.y;

        optional<Event> ev = app.pollEvent();
        while (ev.has_value())
        {
            if (ev->is<Event::Closed>())
            {
                app.close();
            }

            const Event::KeyPressed* keyEv = ev->getIf<Event::KeyPressed>();
            if (keyEv != nullptr)
            {
                handleKeyPress(keyEv->code);
            }

            const Event::MouseButtonPressed* mouseEv = ev->getIf<Event::MouseButtonPressed>();
            if (mouseEv != nullptr)
            {
                curMouseX = (float)mouseEv->position.x;
                curMouseY = (float)mouseEv->position.y;
                handleMouseClick(mouseEv->button == Mouse::Button::Left);
            }

            ev = app.pollEvent();
        }

        app.clear(Color(10, 20, 42));

        switch (currentScreen)
        {
        case SCR_SETUP:
            renderSetup(elapsed);
            break;
        case SCR_FIGHT:
            renderBattle(elapsed);
            break;
        }

        app.display();
    }

    return 0;
}