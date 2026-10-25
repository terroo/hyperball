#include "hyperball.hpp"
#include "game.hpp"
#include "pitch.hpp"
#include "renderer.hpp"

void Hyperball::collide(Body& a, Body& b, float e) {
  sf::Vector2f d = b.pos - a.pos;
  float dist = len(d), minD = a.r + b.r;
  if (dist >= minD) return;
  if (dist < 1e-4f) { b.pos.x += 0.01f; return; }
  sf::Vector2f n = d / dist;
  float ia = 1 / a.mass, ib = 1 / b.mass, sum = ia + ib;
  float overlap = minD - dist;
  a.pos -= n * (overlap * ia / sum);
  b.pos += n * (overlap * ib / sum);
  float vn = dot(b.vel - a.vel, n);
  if (vn < 0) {
    float j = -(1 + e) * vn / sum;
    a.vel -= n * (j * ia);
    b.vel += n * (j * ib);
  }
}

void Hyperball::collideStatic(Body& a, sf::Vector2f c, float cr, float e) {
  sf::Vector2f d = a.pos - c;
  float dist = len(d), minD = a.r + cr;
  if (dist >= minD || dist < 1e-4f) return;
  sf::Vector2f n = d / dist;
  a.pos += n * (minD - dist);
  float vn = dot(a.vel, n);
  if (vn < 0) a.vel -= n * ((1 + e) * vn);
}

void Hyperball::addQuadLine(sf::VertexArray& va, sf::Vector2f a, sf::Vector2f b, float t, sf::Color c) {
  sf::Vector2f d = norm(b - a), n(-d.y * t / 2, d.x * t / 2);
  va.append({a + n, c}); va.append({b + n, c});
  va.append({b - n, c}); va.append({a - n, c});
}

void Hyperball::addArc(sf::VertexArray& va, sf::Vector2f c, float r, float a0, float a1, float t, sf::Color col) {
  const int N = 48;
  sf::Vector2f prev = c + sf::Vector2f(std::cos(a0), std::sin(a0)) * r;
  for (int i = 1; i <= N; ++i) {
    float a = a0 + (a1 - a0) * static_cast<float>(i) / N;
    sf::Vector2f cur = c + sf::Vector2f(std::cos(a), std::sin(a)) * r;
    addQuadLine(va, prev, cur, t, col);
    prev = cur;
  }
}

sf::ConvexShape Hyperball::makeStripe(float r, float halfW) {
  const int N = 10;
  float th1 = std::acos(halfW / r);
  sf::Vector2f n(0.7071f, 0.7071f), d(-0.7071f, 0.7071f);
  sf::ConvexShape s(2 * (N + 1));
  int k = 0;
  for (int half = 0; half < 2; ++half)
    for (int i = 0; i <= N; ++i) {
      float th = (half ? Global::PI : 0.f) + th1 + (Global::PI - 2 * th1) * static_cast<float>(i) / N;
      s.setPoint(static_cast<long unsigned int>(k++), (n * std::cos(th) + d * std::sin(th)) * r);
    }
  s.setFillColor(sf::Color(255, 255, 255, 235));
  return s;
}

sf::String Hyperball::U8(const std::string& s){ 
  return sf::String::fromUtf8(s.begin(), s.end()); 
}


void Hyperball::run(){
  Game game;

  sf::ContextSettings cs; cs.antialiasingLevel = 8;
  sf::RenderWindow window(sf::VideoMode(800, 640), "Hyperball", sf::Style::Default, cs);
  window.setFramerateLimit(120);
  game.reset();
  Renderer renderer;

  sf::Clock clock;
  float acc = 0;
  while (window.isOpen()) {
    sf::Event e;
    while (window.pollEvent(e)) {
      if (e.type == sf::Event::Closed) window.close();
      if (e.type == sf::Event::KeyPressed) {
        if (e.key.code == sf::Keyboard::Escape) window.close();
        if (e.key.code == sf::Keyboard::R) game.reset();
        if (e.key.code == sf::Keyboard::Space && !game.charging && game.state == State::Playing) {
          game.charging = true; game.charge = 0;
        }
      }
      if (e.type == sf::Event::KeyReleased && e.key.code == sf::Keyboard::Space) game.releaseShot();
    }
    float dt = std::min(clock.restart().asSeconds(), 0.05f);
    acc += dt;
    while (acc >= Global::DT) { game.step(Global::DT); acc -= Global::DT; }
    game.hintTimer = std::max(0.f, game.hintTimer - dt);
    renderer.render(window, game, dt);
    window.display();
  }
}
