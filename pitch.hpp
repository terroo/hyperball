struct Pitch {
  sf::VertexArray grass{sf::Quads}, lines{sf::Quads}, net{sf::Quads}, grid{sf::Lines};

  Pitch() {
    using namespace Global;
    const int stripes = 18;
    for (int i = 0; i < stripes; ++i) {
      sf::Color c = (i % 2) ? sf::Color(48, 128, 58) : sf::Color(43, 120, 53);
      float x0 = W * (static_cast<float>(i) / stripes);
      float x1 = W * (static_cast<float>(i + 1) / stripes);
      grass.append({{x0, 0}, c}); grass.append({{x1, 0}, c});
      grass.append({{x1, H}, c}); grass.append({{x0, H}, c});
    }
    const sf::Color L(255, 255, 255, 110);
    const float t = 3.f;
    Hyperball::addQuadLine(lines, {0, 0}, {W, 0}, t, L);
    Hyperball::addQuadLine(lines, {W, 0}, {W, H}, t, L);
    Hyperball::addQuadLine(lines, {W, H}, {0, H}, t, L);
    Hyperball::addQuadLine(lines, {0, H}, {0, 0}, t, L);
    Hyperball::addQuadLine(lines, {W / 2, 0}, {W / 2, H}, t, L);
    Hyperball::addArc(lines, {W / 2, H / 2}, 130.f, 0, 2 * PI, t, L);
    for (int s = 0; s < 2; ++s) {
      float sx = s == 0 ? 1.f : -1.f, x0 = s == 0 ? 0.f : W;
      auto X = [&](float d) { return x0 + sx * d; };
      Hyperball::addQuadLine(lines, {x0, 250}, {X(270), 250}, t, L);
      Hyperball::addQuadLine(lines, {X(270), 250}, {X(270), 750}, t, L);
      Hyperball::addQuadLine(lines, {X(270), 750}, {x0, 750}, t, L);
      Hyperball::addQuadLine(lines, {x0, 330}, {X(100), 330}, t, L);
      Hyperball::addQuadLine(lines, {X(100), 330}, {X(100), 670}, t, L);
      Hyperball::addQuadLine(lines, {X(100), 670}, {x0, 670}, t, L);
      float phi = std::acos(80.f / 120.f);
      if (s == 0) Hyperball::addArc(lines, {190, H / 2}, 120.f, -phi, phi, t, L);
      else        Hyperball::addArc(lines, {W - 190, H / 2}, 120.f, PI - phi, PI + phi, t, L);
    }
    const sf::Color N(18, 20, 22);
    for (int s = 0; s < 2; ++s) {
      float x0 = s == 0 ? -GOAL_D : W, x1 = x0 + GOAL_D;
      net.append({{x0, GY0}, N}); net.append({{x1, GY0}, N});
      net.append({{x1, GY1}, N}); net.append({{x0, GY1}, N});
      const sf::Color G(255, 255, 255, 38);
      for (float x = x0; x <= x1; x += 14.f) { grid.append({{x, GY0}, G}); grid.append({{x, GY1}, G}); }
      for (float y = GY0; y <= GY1; y += 14.f) { grid.append({{x0, y}, G}); grid.append({{x1, y}, G}); }
    }
  }
};
