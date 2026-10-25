#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <deque>
#include <random>
#include <string>
#include <vector>

enum class State { Kickoff, Playing, GoalPause, Over };

struct Body {
  sf::Vector2f pos{0, 0}, vel{0, 0};
  float r = 10, mass = 1;
};

struct Player : Body {
  int team = 0;
  int slot = 0;
  float kickCd = 0;
};

namespace Global {
  const float W = 1800.f, H = 1000.f; 
  const float GOAL_W = 270.f, GOAL_D = 90.f;
  const float GY0 = H / 2 - GOAL_W / 2;
  const float GY1 = H / 2 + GOAL_W / 2;
  const float POST_R = 7.f;
  const float PLAYER_R = 26.f, BALL_R = 11.f;
  const float DT = 1.f / 120.f;
  const float MATCH_TIME = 120.f;
  const float VIEW_H = 600.f;
  const float PI = 3.14159265f;
};

class Hyperball {

  public:


    static float len(sf::Vector2f v) { return std::sqrt(v.x * v.x + v.y * v.y); }
    static sf::Vector2f norm(sf::Vector2f v) { float l = len(v); return l > 1e-5f ? v / l : sf::Vector2f(0, 0); }
    static float dot(sf::Vector2f a, sf::Vector2f b) { return a.x * b.x + a.y * b.y; }

    static void collide(Body& a, Body& b, float e);
    static void collideStatic(Body& a, sf::Vector2f c, float cr, float e);
    static void addQuadLine(sf::VertexArray& va, sf::Vector2f a, sf::Vector2f b, float t, sf::Color c);
    static void addArc(sf::VertexArray& va, sf::Vector2f c, float r, float a0, float a1, float t, sf::Color col);

    static sf::ConvexShape makeStripe(float r, float halfW);
    static sf::String U8(const std::string& s);

    static void run();
};
