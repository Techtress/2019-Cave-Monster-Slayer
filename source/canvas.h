#ifndef CANVAS_H
#define CANVAS_H

#include <list>
#include <map>
#include <vector>

#include "vector2.h"
#include "backdrop.h"

class Entity;
class Image;

// Remember me? (*cough* Project 2 *cough*)
class Canvas
{
 public:
  static Canvas &getInstance();

  enum LayerMap
  {
    LAYER_B4 = -4,
    LAYER_B3 = -3,
    LAYER_B2 = -2,
    LAYER_B1 = -1,
    LAYER_MAIN = 0,
    LAYER_F1 = 1,
    LAYER_F2 = 2,
    LAYER_F3 = 3
  };

  // Draws all entities in layer order
  //  In this order:
  //    entities::draw
  //    backdrops::draw
  void draw() const;

  // Removes all layers and their contents
  void clear() { layers.clear(); layers[0]; }
  void clearLayer(int layerID) { layers.erase(layerID); layers[0]; }

  //
  // Add to layer methods
  //

  void registerEntity(int layerID, const Entity* e) { layers[layerID].entities.push_back(e); }
  void updateEntityList();

  // Creates a beautiful piece of scenery to add to a canvas layer
  Backdrop *placeBackdrop(int layerID, const Image*, const Vec2f &position, int frame, float angle, float scaleX, float scaleY, bool flipH, bool flipV);
  std::list<Backdrop> &getBackdropList(int layerID) { return layers[layerID].backdrops; }

  // (usually just for the backmost layer (is that a word??))
  void setBackground(int layerID, const Image* img) { layers[layerID].background = img; }
  const Image *getBackground(int layerID) const { return layers.at(layerID).background; }
  void setBackgroundAlpha(int layerID, float a) { layers[layerID].bgAlpha = a; }
  float getBackgroundAlpha(int layerID) const { return layers.at(layerID).bgAlpha; }

  // Parallax goodness
  float getScrollFactor(int layerID) const { return layers.at(layerID).scroll; }
  void setScrollFactor(int layerID, float s) { layers[layerID].scroll = s; }

  bool hasLayer(int layerID) const { return layers.find(layerID) != layers.end(); }

  void hideLayer(int layerID) { layers[layerID].visible = false; }
  void showLayer(int layerID) { layers[layerID].visible = true; }

  std::vector<int> getLayerList() const
  {
    std::vector<int> list;
    for (const auto &l : layers)
      list.push_back(l.first);
    return list;
  }

  // Remove from layer (implementation when needed...)
  //void removeEntity(const Entity*);
  //void clearBackdrops();
  //void clearLayer(int layerID)
  //void clearAllLayers();

  Canvas(const Canvas&) = delete;
  Canvas &operator=(const Canvas&) = delete;
  
 private:
  Canvas();

  // The drawing order for backdrops and entities, as well as use for collision detection (main layer only)
  struct CanvasLayer
  {
  CanvasLayer() : background(nullptr), bgAlpha(1.f), backdrops(), entities(), scroll(1.f), visible(true) {}
    CanvasLayer(const CanvasLayer&) = delete;
    CanvasLayer &operator=(const CanvasLayer&) = delete;

    const Image *background;
    float bgAlpha;
    std::list<Backdrop> backdrops;
    std::list<const Entity*> entities;
    float scroll;

    bool visible;
  };
  
  std::map<int, CanvasLayer> layers;
};

#endif
