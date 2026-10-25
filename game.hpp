struct Game {
  Body ball;
  std::vector<Player> players;
  int score[2] = {0, 0};
  float timeLeft = Global::MATCH_TIME;
  State state = State::Kickoff;
  float stateTimer = 0;
  int lastScorer = -1;
  int controlled = -1; 
  bool humanEnabled = true;
  bool charging = false;
  float charge = 0;
  float hintTimer = 6.f;
  float stillTimer = 0;
  std::mt19937 rng{std::random_device{}()};
  int startRed = 0, startBlue = 0;

  float rnd(float a, float b) { return std::uniform_real_distribution<float>(a, b)(rng); }

  void reset() {
    score[0] = startRed; score[1] = startBlue;
    timeLeft = Global::MATCH_TIME;
    hintTimer = 6.f;
    players.assign(6, Player());
    for (int i = 0; i < 6; ++i) {
      players[static_cast<long unsigned int>(i)].team = static_cast<long unsigned int>(i / 3);
      players[static_cast<long unsigned int>(i)].slot = static_cast<long unsigned int>(i % 3);
      players[static_cast<long unsigned int>(i)].r = Global::PLAYER_R;
      players[static_cast<long unsigned int>(i)].mass = 6;
    }
    kickoff();
  }

  void kickoff() {
    using namespace Global;
    const sf::Vector2f red[3] = {{120, H / 2}, {700, H / 2 - 130}, {700, H / 2 + 130}};
    for (int i = 0; i < 3; ++i) {
      players[static_cast<long unsigned int>(i)].pos = red[static_cast<long unsigned int>(i)];
      players[3 + static_cast<long unsigned int>(i)].pos = {W - red[static_cast<long unsigned int>(i)].x, red[static_cast<long unsigned int>(i)].y};
    }
    for (auto& p : players) { p.vel = {0, 0}; p.kickCd = 0; }
    ball.pos = {W / 2, H / 2};
    ball.vel = {0, 0};
    ball.r = BALL_R; ball.mass = 1.f;
    charging = false; charge = 0;
    state = State::Kickoff;
    stateTimer = 1.0f;
    controlled = humanEnabled ? nearestRed() : -1;
  }

  int nearestRed() const {
    int best = 0; float bd = 1e9f;
    for (int i = 0; i < 3; ++i) {
      float d = Hyperball::len(players[static_cast<long unsigned int>(i)].pos - ball.pos);
      if (d < bd) { bd = d; best = static_cast<long unsigned int>(i); }
    }
    return best;
  }

  bool tryKick(Player& p, sf::Vector2f dir, float power) {
    float reach = p.r + ball.r + 14.f;
    if (p.kickCd > 0 || Hyperball::len(ball.pos - p.pos) > reach) return false;
    ball.vel = Hyperball::norm(dir) * power;
    p.kickCd = 0.30f;
    return true;
  }

  sf::Vector2f humanInput() const {
    sf::Vector2f d(0, 0);
    using K = sf::Keyboard;
    if (K::isKeyPressed(K::A) || K::isKeyPressed(K::Left))  d.x -= 1;
    if (K::isKeyPressed(K::D) || K::isKeyPressed(K::Right)) d.x += 1;
    if (K::isKeyPressed(K::W) || K::isKeyPressed(K::Up))    d.y -= 1;
    if (K::isKeyPressed(K::S) || K::isKeyPressed(K::Down))  d.y += 1;
    return Hyperball::norm(d);
  }

  void releaseShot() {
    if (!charging) return;
    charging = false;
    if (state != State::Playing || controlled < 0) { charge = 0; return; }
    Player& p = players[static_cast<long unsigned int>(controlled)];
    sf::Vector2f toBall = Hyperball::norm(ball.pos - p.pos);
    sf::Vector2f dir = Hyperball::norm(toBall + humanInput() * 0.6f);
    if (Hyperball::len(dir) < 0.1f) dir = toBall;
    tryKick(p, dir, 480.f + 720.f * charge);
    charge = 0;
  }

  sf::Vector2f aiMove(Player& p, bool isChaser) {
    using namespace Global;
    const float a = p.team == 0 ? 1.f : -1.f;       // sentido do ataque
    const sf::Vector2f goal(p.team == 0 ? W : 0.f, H / 2);
    const sf::Vector2f ownGoal(p.team == 0 ? 0.f : W, H / 2);
    sf::Vector2f target;
    bool nearOwnGoal = Hyperball::len(ball.pos - ownGoal) < 330.f;

    if ((p.slot == 0 && !nearOwnGoal) || (!isChaser && p.slot == 0)) {
      target.x = ownGoal.x + a * 120.f;
      target.y = H / 2 + (ball.pos.y - H / 2) * 0.45f;
      target.y = std::clamp(target.y, GY0 - 30.f, GY1 + 30.f);
    } else if (isChaser || (p.slot == 0 && nearOwnGoal)) {
      sf::Vector2f toGoal = goal - ball.pos;
      toGoal.y += (H / 2 - ball.pos.y) * 1.2f;
      toGoal = Hyperball::norm(toGoal);
      float d = Hyperball::len(ball.pos - p.pos);
      bool ballAhead = Hyperball::dot(ball.pos - p.pos, toGoal) > 0;
      bool oppCloser = false;
      for (auto& o : players)
        if (o.team != p.team && Hyperball::len(o.pos - ball.pos) < d * 0.9f) oppCloser = true;

      if (!ballAhead && d > 110.f && !oppCloser) {
        target = ball.pos - toGoal * (p.r + ball.r + 8.f);
        bool outside = target.x < p.r || target.x > W - p.r || target.y < p.r || target.y > H - p.r;
        if (outside) 
          target = ball.pos + Hyperball::norm(sf::Vector2f(W / 2, H / 2) - ball.pos) * (p.r + ball.r + 6.f);
      } else if (!ballAhead) {
        target = ball.pos + ball.vel * 0.2f;
      } else {
        target = ball.pos + toGoal * 6.f;
      }
      // chute
      if (state == State::Playing && d < p.r + ball.r + 12.f) {
        if (ballAhead) {
          float distGoal = Hyperball::len(goal - ball.pos);
          sf::Vector2f dir = toGoal;
          float power = rnd(420.f, 620.f);
          if (distGoal < 650.f) {
            sf::Vector2f aim(goal.x, H / 2 + rnd(-95.f, 95.f));
            dir = aim - ball.pos;
            power = rnd(700.f, 950.f);
          }
          tryKick(p, dir, power);
        } else {
          tryKick(p, Hyperball::norm(ball.pos - p.pos) + toGoal * 0.6f, 520.f);
        }
      }
    } else {
      float side = p.slot == 1 ? -1.f : 1.f;
      target.x = std::clamp(ball.pos.x + a * 190.f, 160.f, W - 160.f);
      target.y = std::clamp(ball.pos.y + side * 190.f, 90.f, H - 90.f);
    }
    sf::Vector2f d = target - p.pos;
    float l = Hyperball::len(d);
    return l < 6.f ? sf::Vector2f(0, 0) : d / std::max(l, 40.f) * std::min(1.f, l / 40.f);
  }

  void step(float dt) {
    using namespace Global;
    if (state == State::Kickoff) {
      stateTimer -= dt;
      if (stateTimer <= 0) state = State::Playing;
    } else if (state == State::Playing) {
      timeLeft -= dt;
      if (timeLeft <= 0) { timeLeft = 0; state = State::Over; }
    } else if (state == State::GoalPause) {
      stateTimer -= dt;
      if (stateTimer <= 0) kickoff();
    }
    for (auto& p : players) p.kickCd = std::max(0.f, p.kickCd - dt);

    if (humanEnabled && controlled >= 0 && state != State::Over) {
      int n = nearestRed();
      float dn = Hyperball::len(players[static_cast<long unsigned int>(n)].pos - ball.pos);
      float dc = Hyperball::len(players[static_cast<long unsigned int>(controlled)].pos - ball.pos);
      if (n != controlled && dn < dc - 25.f && !charging) controlled = n;
    }
    if (charging) charge = std::min(1.f, charge + dt / 0.6f);

    int chaser[2] = {-1, -1};
    for (int t = 0; t < 2; ++t) {
      float bd = 1e9f;
      for (int i = 0; i < 3; ++i) {
        int idx = t * 3 + i;
        if (idx == controlled || i == 0) continue;
        float d = Hyperball::len(players[static_cast<long unsigned int>(idx)].pos - ball.pos);
        if (d < bd) { bd = d; chaser[t] = idx; }
      }
    }

    bool active = state == State::Playing;
    for (int i = 0; i < 6; ++i) {
      Player& p = players[static_cast<long unsigned int>(i)];
      sf::Vector2f dir(0, 0);
      float vmax = p.team == 0 ? 285.f : 270.f;
      if (active) {
        if (i == controlled) { dir = humanInput(); vmax = 305.f; if (charging) vmax *= 0.85f; }
        else dir = aiMove(p, i == chaser[p.team]);
      }
      p.vel += (dir * vmax - p.vel) * std::min(1.f, 9.f * dt);
    }

    for (auto& p : players) p.pos += p.vel * dt;
    ball.pos += ball.vel * dt;
    ball.vel *= std::exp(-0.7f * dt);
    if (Hyperball::len(ball.vel) < 3.f) ball.vel = {0, 0};

    for (int i = 0; i < 6; ++i)
      for (int j = i + 1; j < 6; ++j) Hyperball::collide(players[static_cast<long unsigned int>(i)], players[static_cast<long unsigned int>(j)], 0.3f);
    for (auto& p : players) Hyperball::collide(p, ball, 0.55f);

    const sf::Vector2f posts[4] = {{0, GY0}, {0, GY1}, {W, GY0}, {W, GY1}};
    for (auto& po : posts) {
      Hyperball::collideStatic(ball, po, POST_R, 0.8f);
      for (auto& p : players) Hyperball::collideStatic(p, po, POST_R, 0.2f);
    }

    for (auto& p : players) {
      if (p.pos.x < p.r) { p.pos.x = p.r; p.vel.x = 0; }
      if (p.pos.x > W - p.r) { p.pos.x = W - p.r; p.vel.x = 0; }
      if (p.pos.y < p.r) { p.pos.y = p.r; p.vel.y = 0; }
      if (p.pos.y > H - p.r) { p.pos.y = H - p.r; p.vel.y = 0; }
    }
    ballWalls();

    if (state == State::Playing) {
      float nearest = 1e9f;
      for (auto& p : players) nearest = std::min(nearest, Hyperball::len(p.pos - ball.pos));
      if (Hyperball::len(ball.vel) < 35.f && nearest < ball.r + Global::PLAYER_R + 20.f) stillTimer += dt;
      else stillTimer = 0;
      if (stillTimer > 1.2f) {
        sf::Vector2f toC = Hyperball::norm(sf::Vector2f(W / 2, H / 2) - ball.pos);
        ball.pos += toC * 55.f;
        ball.vel = toC * 380.f;
        stillTimer = 0;
      }
    } else stillTimer = 0;

    if (state == State::Playing) {
      int scorer = -1;
      if (ball.pos.x < -ball.r) scorer = 1;
      else if (ball.pos.x > W + ball.r) scorer = 0;
      if (scorer >= 0) {
        score[scorer]++;
        lastScorer = scorer;
        state = State::GoalPause;
        stateTimer = 2.2f;
        charging = false; charge = 0;
      }
    }
  }

  void ballWalls() {
    using namespace Global;
    Body& b = ball;
    const float e = 0.75f, r = b.r;
    if (b.pos.y < r) { b.pos.y = r; b.vel.y = std::abs(b.vel.y) * e; }
    if (b.pos.y > H - r) { b.pos.y = H - r; b.vel.y = -std::abs(b.vel.y) * e; }
    bool mouth = b.pos.y > GY0 && b.pos.y < GY1;

    auto sideNet = [&]() {
      if (b.pos.y < GY0 + r) { b.pos.y = GY0 + r; b.vel.y = std::abs(b.vel.y) * e; }
      if (b.pos.y > GY1 - r) { b.pos.y = GY1 - r; b.vel.y = -std::abs(b.vel.y) * e; }
    };
    if (b.pos.x < r) {
      if (mouth) {
        if (b.pos.x < -GOAL_D + r) { b.pos.x = -GOAL_D + r; b.vel.x = std::abs(b.vel.x) * e; }
        if (b.pos.x < 0) sideNet();
      } else { b.pos.x = r; b.vel.x = std::abs(b.vel.x) * e; }
    }
    if (b.pos.x > W - r) {
      if (mouth) {
        if (b.pos.x > W + GOAL_D - r) { b.pos.x = W + GOAL_D - r; b.vel.x = -std::abs(b.vel.x) * e; }
        if (b.pos.x > W) sideNet();
      } else { b.pos.x = W - r; b.vel.x = -std::abs(b.vel.x) * e; }
    }
  }
};
