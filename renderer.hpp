struct Renderer {
  sf::Font font;
  bool hasFont = false;
  Pitch pitch;
  sf::ConvexShape stripe = Hyperball::makeStripe(Global::PLAYER_R - 2, 6.5f);
  std::deque<sf::Vector2f> trail;
  sf::Vector2f cam{Global::W / 2, Global::H / 2};

  Renderer() {
    const char* paths[] = {
      "font.ttf", "assets/font.ttf",
      "C:/Windows/Fonts/impact.ttf", "C:/Windows/Fonts/arialbd.ttf",
      "/System/Library/Fonts/Supplemental/Impact.ttf",
      "/System/Library/Fonts/Supplemental/Arial Bold.ttf",
      "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
      "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
      "/usr/share/fonts/TTF/DejaVuSans-Bold.ttf"};
    for (auto p : paths) if (font.loadFromFile(p)) { hasFont = true; break; }
    if (!hasFont) std::fprintf(stderr, "Aviso: nenhuma fonte encontrada. Coloque um 'font.ttf' no diretorio do executável.\n");
  }

  void text(sf::RenderTarget& w, const std::string& s, sf::Vector2f pos, unsigned size, sf::Color fill,
      bool center = false, float outline = 3.f, sf::Color oc = sf::Color(15, 18, 22)) {
    if (!hasFont) return;
    sf::Text t(Hyperball::U8(s), font, size);
    t.setFillColor(fill);
    t.setOutlineColor(oc);
    t.setOutlineThickness(outline);
    if (center) { auto b = t.getLocalBounds(); t.setOrigin(b.left + b.width / 2, b.top + b.height / 2); }
    t.setPosition(pos);
    w.draw(t);
  }

  void banner(sf::RenderWindow& w, const std::string& s, float y, sf::Color bg, sf::Color fg, unsigned size) {
    if (!hasFont) return;
    sf::Text t(Hyperball::U8(s), font, size);
    auto b = t.getLocalBounds();
    sf::RectangleShape r({b.width + 36.f, static_cast<float>(size) * 1.35f});
    r.setOrigin(r.getSize() / 2.f);
    r.setPosition(static_cast<float>(w.getSize().x) / 2.f, y);
    r.setFillColor(bg);
    w.draw(r);
    text(w, s, {static_cast<float>(w.getSize().x) / 2.f, y}, size, fg, true, 0.f);
  }

  void drawPlayer(sf::RenderTarget& w, const Player& p, bool ctrl) {
    const float r = p.r - 2.f;
    sf::CircleShape sh(r); sh.setOrigin(r, r);
    sh.setPosition(p.pos + sf::Vector2f(4, 6)); sh.setFillColor(sf::Color(0, 0, 0, 60)); w.draw(sh);

    sf::CircleShape c(r); c.setOrigin(r, r); c.setPosition(p.pos);
    c.setFillColor(p.team == 0 ? sf::Color(244, 84, 44) : sf::Color(40, 98, 214));
    c.setOutlineThickness(3.f); c.setOutlineColor(sf::Color(20, 24, 30));
    w.draw(c);
    stripe.setPosition(p.pos); w.draw(stripe);
    if (ctrl) {
      sf::CircleShape ring(r + 9.f); ring.setOrigin(r + 9.f, r + 9.f); ring.setPosition(p.pos);
      ring.setFillColor(sf::Color::Transparent);
      ring.setOutlineThickness(2.5f); ring.setOutlineColor(sf::Color(255, 220, 60, 220));
      w.draw(ring);
    }
  }

  void render(sf::RenderWindow& w, Game& g, float frameDt) {
    using namespace Global;
    sf::Vector2f ws(static_cast<float>(w.getSize().x), static_cast<float>(w.getSize().y));
    float aspect = ws.x / ws.y;
    sf::View view; view.setSize(VIEW_H * aspect, VIEW_H);

    sf::Vector2f target = g.ball.pos + g.ball.vel * 0.22f;
    cam += (target - cam) * (1.f - std::exp(-4.5f * frameDt));
    float mx = GOAL_D + 40.f, my = 40.f;
    float hw = view.getSize().x / 2, hh = view.getSize().y / 2;
    sf::Vector2f c = cam;
    c.x = (view.getSize().x >= W + 2 * mx) ? W / 2 : std::clamp(c.x, -mx + hw, W + mx - hw);
    c.y = (view.getSize().y >= H + 2 * my) ? H / 2 : std::clamp(c.y, -my + hh, H + my - hh);
    view.setCenter(c);

    w.clear(sf::Color(14, 16, 18));
    w.setView(view);
    w.draw(pitch.grass);
    w.draw(pitch.net);
    w.draw(pitch.grid);
    w.draw(pitch.lines);

    sf::CircleShape dot(4); dot.setOrigin(4, 4); dot.setFillColor(sf::Color(255, 255, 255, 130));
    for (sf::Vector2f p : {sf::Vector2f(W / 2, H / 2), sf::Vector2f(190, H / 2), sf::Vector2f(W - 190, H / 2)}) { dot.setPosition(p); w.draw(dot); }

    float speed = Hyperball::len(g.ball.vel);
    trail.push_back(g.ball.pos);
    if (trail.size() > 16 || speed < 200.f) { if (!trail.empty()) trail.pop_front(); }
    if (speed < 200.f && trail.size() > 1) trail.pop_front();
    for (size_t i = 1; i < trail.size(); ++i) {
      float k = static_cast<float>(i) / static_cast<float>(trail.size());
      sf::VertexArray q(sf::Quads);
      Hyperball::addQuadLine(q, trail[i - 1], trail[i], 3.f + 14.f * k, sf::Color(255, 255, 255, (sf::Uint8)(70 * k)));
      w.draw(q);
    }

    for (int i = 0; i < 6; ++i){
      drawPlayer(w, g.players[static_cast<long unsigned int>(i)], i == g.controlled);
    }

    {
      const float r = g.ball.r;
      sf::CircleShape sh(r); sh.setOrigin(r, r); sh.setPosition(g.ball.pos + sf::Vector2f(3, 4));
      sh.setFillColor(sf::Color(0, 0, 0, 70)); w.draw(sh);
      sf::CircleShape b(r - 1.5f); b.setOrigin(r - 1.5f, r - 1.5f); b.setPosition(g.ball.pos);
      b.setFillColor(sf::Color(250, 250, 250)); b.setOutlineThickness(2.f); b.setOutlineColor(sf::Color(25, 28, 34));
      w.draw(b);
      sf::CircleShape pent(4.5f, 5); pent.setOrigin(4.5f, 4.5f); pent.setPosition(g.ball.pos);
      pent.setFillColor(sf::Color(25, 28, 34)); w.draw(pent);
    }

    sf::CircleShape post(POST_R); post.setOrigin(POST_R, POST_R);
    post.setFillColor(sf::Color(245, 245, 245)); post.setOutlineThickness(1.5f);
    post.setOutlineColor(sf::Color(0, 0, 0, 90));
    for (sf::Vector2f p : {sf::Vector2f(0, GY0), sf::Vector2f(0, GY1), sf::Vector2f(W, GY0), sf::Vector2f(W, GY1)}) { post.setPosition(p); w.draw(post); }

    if (g.charging && g.controlled >= 0) {
      sf::Vector2f p = g.players[static_cast<long unsigned int>(g.controlled)].pos + sf::Vector2f(0, g.players[static_cast<long unsigned int>(g.controlled)].r + 16.f);
      sf::RectangleShape bg({56.f, 8.f}); bg.setOrigin(28.f, 4.f); bg.setPosition(p);
      bg.setFillColor(sf::Color(0, 0, 0, 150)); w.draw(bg);
      sf::RectangleShape fg({52.f * g.charge, 4.f}); fg.setPosition(p + sf::Vector2f(-26.f, -2.f));
      fg.setFillColor(sf::Color(255, 190, 40)); w.draw(fg);
    }

    drawHud(w, g);
  }

  void drawHud(sf::RenderWindow& w, const Game& g) {
    w.setView(sf::View(sf::FloatRect(0, 0, (float)w.getSize().x, (float)w.getSize().y)));
    float W_ = (float)w.getSize().x, H_ = (float)w.getSize().y;
    float u = H_ / 600.f;

    sf::RectangleShape top({W_, 4.f}); top.setFillColor(sf::Color(255, 90, 30)); w.draw(top);
    text(w, "HYPERBALL", {14 * u, 12 * u}, (unsigned)(18 * u), sf::Color::White);

    float bw = 34 * u, bh = 28 * u, gap = 18 * u;
    float cx = W_ - 14 * u - (bw * 2 + gap) / 2;
    sf::RectangleShape bgr({bw * 2 + gap + 10 * u, bh + 10 * u});
    bgr.setOrigin(bgr.getSize() / 2.f); bgr.setPosition(cx, 14 * u + bh / 2);
    bgr.setFillColor(sf::Color(18, 22, 28, 235)); w.draw(bgr);
    sf::RectangleShape r1({bw, bh}), r2({bw, bh});
    r1.setFillColor(sf::Color(244, 84, 44)); r2.setFillColor(sf::Color(40, 98, 214));
    r1.setOrigin(bw / 2, bh / 2); r2.setOrigin(bw / 2, bh / 2);
    r1.setPosition(cx - (bw + gap) / 2, 14 * u + bh / 2); r2.setPosition(cx + (bw + gap) / 2, 14 * u + bh / 2);
    w.draw(r1); w.draw(r2);
    text(w, std::to_string(g.score[0]), r1.getPosition(), (unsigned)(20 * u), sf::Color::White, true, 0);
    text(w, "-", {cx, 14 * u + bh / 2}, (unsigned)(18 * u), sf::Color(200, 205, 215), true, 0);
    text(w, std::to_string(g.score[1]), r2.getPosition(), (unsigned)(20 * u), sf::Color::White, true, 0);

    int secs = (int)std::ceil(g.timeLeft);
    char buf[16]; std::snprintf(buf, sizeof buf, "%d:%02d", secs / 60, secs % 60);
    text(w, buf, {W_ / 2, 24 * u}, (unsigned)(22 * u), sf::Color::White, true);

    if (g.state == State::GoalPause) {
      bool red = g.lastScorer == 0;
      banner(w, red ? "GOOOL DO VERMELHO!" : "GOL DO AZUL!", H_ * 0.82f,
          red ? sf::Color(244, 84, 44) : sf::Color(40, 98, 214), sf::Color::White, (unsigned)(30 * u));
    } else if (g.state == State::Over) {
      std::string res = g.score[0] > g.score[1] ? "VOCÊ VENCEU!" : g.score[0] < g.score[1] ? "VOCÊ PERDEU." : "EMPATE.";
      banner(w, "FIM DE JOGO", H_ * 0.40f, sf::Color(18, 22, 28, 235), sf::Color::White, (unsigned)(34 * u));
      banner(w, res, H_ * 0.52f, sf::Color(255, 90, 30), sf::Color::White, (unsigned)(30 * u));
      text(w, "Aperte R para jogar de novo", {W_ / 2, H_ * 0.64f}, (unsigned)(18 * u), sf::Color::White, true);
    } else if (g.hintTimer > 0 && g.humanEnabled) {
      text(w, "WASD/Setas: mover    Espaço: chutar (segure p/ força)", {W_ / 2, H_ - 28 * u},
          (unsigned)(15 * u), sf::Color::White, true, 2.f);
    }
  }
};
