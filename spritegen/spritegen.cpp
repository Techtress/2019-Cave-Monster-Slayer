#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <stack>
#include <set>
#include <map>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#define PNG_DEBUG 3
#include <png.h>

#include "../source/vector2.h"

struct Image
{
  int width, height;
  png_bytep *row_pointers;

  ~Image() {

    for (int y=0; y<height; y++)
      free(row_pointers[y]);
    free(row_pointers);
  }

  int offset_left, offset_right;
};


using namespace std;



png_byte color_type;
png_byte bit_depth;

png_structp png_ptr;
png_infop info_ptr;
int number_of_passes;

string imageName(string prefix, int i);
Image *readImage( string filename );
void writeImage( Image* img, string filename );

string spaces = "";
stack<string> tags;

string toStr(int val)
{
  stringstream ss;
  ss << val;
  return ss.str();
}

void openTag(ostream& o, const string& tagName, const vector<pair<string, string>>& variables, bool end = false)
{
  o << spaces << "<" << tagName;
  for (const auto &var : variables)
    o << " " << var.first << "=\"" << var.second << "\"";
  if (end) o << " /";
  else {
    spaces = spaces + "  ";
    tags.push(tagName);
  }
  o << ">\n";
}

void writeVariable(ostream& o, const string &tagName, const string &variable)
{
  o << spaces << "<" << tagName << ">" << variable << "</" << tagName << ">\n";
}

void closeTag(ostream& o)
{
  spaces.resize(spaces.length()-2);
  o << spaces << "</" << tags.top() << ">\n";
  tags.pop();
}

struct rectangle
{
  unsigned short x, y, w, h;
  short ox, oy, cx, cy;
  unsigned img_id;
};

struct vertex
{
  float x, y;
};

struct shape
{
  std::vector<vertex> data;
};

bool rectIntersect( const rectangle &p, const rectangle &q)
{
  if (p.x < q.x+q.w && q.x < p.x+p.w &&
      p.y < q.y+q.h && q.y < p.y+p.h)
    return true;
  return false;
}

bool pointInRect( int x, int y, const rectangle &p)
{
  if (x >= p.x && x < p.x+p.w && y >= p.y && y < p.y+p.h)
    return true;
  return false;
}

bool operator<(const rectangle &p, const rectangle &q)
{
  return p.w*p.h > q.w*q.h;
}

string prefix;
Image *newImage;
std::vector<Image*> images;
multiset<rectangle> rects;
std::vector<shape> shapes;

void readAnimation()
{
  string file = "animations/" + prefix + "/" + prefix;
  unsigned short cx, cy;
  cout << "Please enter image center: " << endl;
  cin >> cx >> cy;
  
  int i = 0;
  
  while (newImage = readImage(imageName(file, i))) {
    i++;
    int offset_left = newImage->width,
      offset_right = 0,
      offset_top = newImage->height,
      offset_bottom = 0;

    // Calculate boundaries to reduce blank area
    for (int y = 0; y < newImage->height; y++) {
      for (int x = 0; x < newImage->width; x++) {
	png_byte *ptr = &(newImage->row_pointers[y][x*4]);
	if (ptr[3] > 0) {
	  offset_left = min(offset_left, x);
	  offset_right = max(offset_right, x);
	  offset_top = min(offset_top, y);
	  offset_bottom = max(offset_bottom, y);
	}
      }
    }

    rects.insert( {0, 0, offset_right-offset_left+1, offset_bottom-offset_top+1, offset_left, offset_top, cx, cy, images.size()} );
    images.push_back(newImage);

    // just add empty shape for now...
    shapes.push_back(shape());
  }
}

void readBackdropSet()
{
  string dir = "backdropsets/" + prefix + "/";
  string file = dir + prefix;

  int i = 0;
  while (newImage = readImage(imageName(file, i))) {
    i++;
    int offset_left = newImage->width,
      offset_right = 0,
      offset_top = newImage->height,
      offset_bottom = 0;

    // Calculate boundaries to reduce blank area
    for (int y = 0; y < newImage->height; y++) {
      for (int x = 0; x < newImage->width; x++) {
	png_byte *ptr = &(newImage->row_pointers[y][x*4]);
	if (ptr[3] > 0) {
	  offset_left = min(offset_left, x);
	  offset_right = max(offset_right, x);
	  offset_top = min(offset_top, y);
	  offset_bottom = max(offset_bottom, y);
	}
      }
    }

    offset_left -= 2;
    offset_top -= 2;
    offset_right += 2;
    offset_bottom += 2;

    rects.insert( {0, 0, offset_right-offset_left+1, offset_bottom-offset_top+1, offset_left, offset_top,
	  newImage->width/2, newImage->height/2, images.size()} );
    images.push_back(newImage);

    // just add empty shape for now...
    shapes.push_back(shape());
  }
}

int main(void)
{
  cout << "Please enter image prefix: " << endl;
  cin >> prefix;
  if (prefix.length() < 3)
    return 0;
  
  cout << "Choose program:\n  0: animation\n  1: backdrop set" << endl;
  int type;
  cin >> type;

  if (type == 0)
    readAnimation();
  else if (type == 1) {
    readBackdropSet();
  }
  else
    return 0;
  
  cout << "Found " << images.size() << " images" << endl;

  if (images.size() == 0)
    return 0;

  bool sorting = true;
  int size = 128;
  vector<rectangle> imgMap(images.size());

  while (sorting) {

    //cout << size << endl << flush;
    
    multiset<rectangle> temp = rects;
    vector<rectangle> fills;

    bool ok = true;

    for (int y = 0; y < size && ok; y++) {
      for (auto itr = temp.begin(); itr != temp.end(); ++itr) {
	rectangle r = *itr;
	if (r.w > size) {
	  ok = false;
	  break;
	}
	if (y + r.h <= size) {
	  //cout << "Fit image " << r.img_id << " at " << "0, " << y << endl << flush;
	  //cout << "  " << temp.size() << endl;
	  fills.push_back({0, y, r.w, r.h, r.ox, r.oy, r.cx, r.cy});
	  imgMap[r.img_id] = fills.back();
	  temp.erase(itr);
	  y += r.h-1;
	  break;
	}
      }
    }
    for (int y = 0; y < size && ok; y++) {
      for (int x = 0; x < size && ok; x++) {
	//if fits
	bool success = false;
	int xtries = 0;

	// Don't check for impossibilities
	for (const rectangle &f : fills) {
	  if (pointInRect( x, y, f )) {
	    x = f.x+f.w;
	    break;
	  }
	}

	if (x >= size)
	  continue;
	
	for (auto itr = temp.begin(); itr != temp.end(); ++itr) {
	  rectangle r = *itr;
	  //cout << "Trying image " << r.img_id << "..." << endl;
	  bool fits = true;
	  if (x+r.w > size) {
	    //cout << " too wide..." << (size-x) << " " << (x+r.w) << endl;
	    if (++xtries == temp.size())
	      break;
	    continue;
	  }
	  if (y + r.h > size) {
	    //cout << " too tall..." << (size-y) << " " << (y+r.h) << endl;
	    ok = false;
	    break;
	  }
	  for (const rectangle &f : fills) {
	    if (rectIntersect( {x, y, r.w, r.h}, f )) {
	      fits = false;
	      break;
	    }
	  }
	  if (fits) {
	    success = true;
	    //cout << "Fit image " << r.img_id << " at " << x << ", " << y << endl << flush;
	    //cout << "  " << temp.size() << endl;
	    fills.push_back({x, y, r.w, r.h, r.ox, r.oy, r.cx, r.cy});
	    imgMap[r.img_id] = fills.back();
	    temp.erase(itr);
	    x += r.w-1;
	    break;
	  }
	}
      }
    }

    // We are done...
    if (temp.size() == 0) sorting = false;
    else size += 64;
  }

  cout << "Final size: " << size << "x" << size << endl;

  ofstream frames(prefix + ".frame", ios::out | ios::binary);
  unsigned short count = imgMap.size();
  frames.write((const char*)&count, sizeof(unsigned short));
  for (size_t i = 0; i < imgMap.size(); i++) {
    const auto &p = imgMap[i];
    shape &s = shapes[i];
    frames.write((const char*)&p, sizeof(unsigned short)*4);
    unsigned short centerx = p.ox-p.cx, centery = p.oy-p.cy;
    frames.write((const char*)&centerx, sizeof(unsigned short));
    frames.write((const char*)&centery, sizeof(unsigned short));

    unsigned size = s.data.size();
    frames.write((const char*)&size, sizeof(unsigned));
    for (vertex &v : s.data) {
      frames.write((const char*)&v, sizeof(float) * 2);
    }
  }

  newImage = new Image;

  newImage->width = newImage->height = size;
  newImage->row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * newImage->height);
  for (int y=0; y< newImage->height; y++)
    newImage->row_pointers[y] = (png_byte*) malloc(sizeof(png_byte*) * newImage->width);

  for (const rectangle &r : rects) {
    int start_x = imgMap[r.img_id].x;
    int start_y = imgMap[r.img_id].y;
    for (int y = 0; y < r.h; y++) {
      for (int x = 0; x < r.w; x++) {
	png_byte *ptr_dest = &(newImage->row_pointers[start_y+y][(start_x+x)*4]);
	png_byte *ptr_src = &(images[r.img_id]->row_pointers[r.oy+y][(r.ox+x)*4]);
	for (int p = 0; p < 4; p++)
	  ptr_dest[p] = ptr_src[p];
      }
    }
  }

  writeImage( newImage, prefix + ".png" );

  cout << "Saved image '" << prefix << ".png' and framedata in '" << prefix << ".frame'\n";

  

  for (Image *img : images) delete img;
  delete newImage;
  
  return 0;
}

string imageName(string prefix, int i)
{
  stringstream ss;
  ss << prefix;
  ss << setfill('0') << setw(4) << i;
  ss << ".png";
  return ss.str();
}

Image *readImage( string filename )
{
  unsigned char header[8];
  FILE *fp = fopen(filename.c_str(), "rb");
  if (!fp) return nullptr;

  fread(header, 1, 8, fp);
  if (png_sig_cmp(header, 0, 8)) return nullptr;

  if (!(png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)))
    return nullptr;
  if (!(info_ptr = png_create_info_struct(png_ptr)))
    return nullptr;
  
  if (setjmp(png_jmpbuf(png_ptr)))
    return nullptr;

  Image *img = new Image;

  png_init_io(png_ptr, fp);
  png_set_sig_bytes(png_ptr, 8);

  png_read_info(png_ptr, info_ptr);

  img->width = png_get_image_width(png_ptr, info_ptr);
  img->height = png_get_image_height(png_ptr, info_ptr);
  
  color_type = png_get_color_type(png_ptr, info_ptr);
  bit_depth = png_get_bit_depth(png_ptr, info_ptr);
  number_of_passes = png_set_interlace_handling(png_ptr);
  
  png_read_update_info(png_ptr, info_ptr);

  /* read file */
  if (setjmp(png_jmpbuf(png_ptr))) {
    delete img;
    return nullptr;
  }

  img->row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * img->height);
  for (int y=0; y<img->height; y++)
    img->row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));

  png_read_image(png_ptr, img->row_pointers);

  fclose(fp);

  return img;
}

void writeImage( Image* img, string filename )
{
  /* create file */
  FILE *fp = fopen(filename.c_str(), "wb");
  if (!fp) return;

  /* initialize stuff */
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) return;

  png_set_compression_level(png_ptr, 9);

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) return;
  if (setjmp(png_jmpbuf(png_ptr))) return;

  png_init_io(png_ptr, fp);

  /* write header */
  if (setjmp(png_jmpbuf(png_ptr))) return;

  png_set_IHDR(png_ptr, info_ptr, img->width, img->height,
	       bit_depth, color_type, PNG_INTERLACE_NONE,
	       PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

  png_write_info(png_ptr, info_ptr);


  /* write bytes */
  if (setjmp(png_jmpbuf(png_ptr))) return;

  png_write_image(png_ptr, img->row_pointers);


  /* end write */
  if (setjmp(png_jmpbuf(png_ptr))) return;

  png_write_end(png_ptr, NULL);

  fclose(fp);
}
