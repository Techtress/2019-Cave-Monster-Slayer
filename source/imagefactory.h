#include <string>
#include <map>

class Image;
class SDL_Surface;
class SDL_Texture;

class ImageFactory {
  
public:
  static ImageFactory& getInstance();
  ~ImageFactory();

  Image* getImage(const std::string&);

  ImageFactory(const ImageFactory&) = delete;
  ImageFactory& operator=(const ImageFactory&) = delete;

private:
  std::map<std::string, SDL_Surface*> surfaces;
  std::map<std::string, SDL_Texture*> textures;
  std::map<std::string, Image*> images;

  ImageFactory() : 
    surfaces(),
    textures(),
    images()
  {}
};
