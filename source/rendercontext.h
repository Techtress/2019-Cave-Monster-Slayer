#ifndef RENDERCONTEXT_H
#define RENDERCONTEXT_H

class SDL_Window;
class SDL_Renderer;

class RenderContext{
public:
  static RenderContext& getInstance();
  ~RenderContext();
  SDL_Window* getWindow() const { return window; }
  SDL_Renderer* getRenderer() const { return renderer; }

  RenderContext(const RenderContext&) = delete;
  RenderContext& operator=(const RenderContext&) = delete;

private:
  SDL_Window* window;
  SDL_Renderer* renderer;

  SDL_Window* initWindow();
  SDL_Renderer* initRenderer();
  RenderContext();
};

#endif
