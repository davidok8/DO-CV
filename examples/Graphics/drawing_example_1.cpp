#include <DO/Graphics.hpp>

using namespace std;
using namespace DO;

int main()
{
  Window W = openWindow(512, 512, "2D basics");

  // Draw a red line from (20, 10) to (300, 100) with 5-pixel thickness.
  drawLine(20, 10, 300, 100, Red8, 5);
  // Draw a black line from (320, 100) to (500, 100) with 5-pixel thickness.
  drawLine(Point2i(320, 100), Point2i(500, 100), Black8, 5);

  // Draw a blue rectangle with top-left corner (400, 10) and size (100, 50).
  drawRect(400, 10, 100, 50, Blue8, 3);
  // Draw a green color-filled rectangle with top-left corner (400, 400) and
  // size (100, 50).
  fillRect(Point2i(400, 400), 100, 50, Green8);

  // Draw an axis-aligned ellipse bounded by a rectangle whose top-left 
  // corner is (50,350) and size is (50, 90) using a cyan Pen with a 2-pixel
  // thickness.
  drawEllipse(50, 350, 50, 90, Cyan8, 2);
  // Simple exercise: decipher this one.
  fillEllipse(350, 150, 90, 100, Color3ub(128, 128, 128));
  // A circle with a center point located at (200, 200) and a 40-pixel radius.
  drawCircle(Point2i(200, 200), 40, Red8);

  /*
   Draw an oriented ellipse with
   - center = (150, 100)
   - radii r1 = 10, r2 = 20,
   - orientation = 45 degree
   - in cyan color,
   - pencil width = 1.
   */
  drawEllipse(Vector2f(150.f, 100.f), 10.f, 20.f, 45.f, Cyan8, 1);
  drawEllipse(Vector2f(50.f, 50.f), 10.f, 20.f, 0.f, Red8, 1);

  // Draw a few black points.
  for (int i = 0; i < 20; i+= 2)
    drawPoint(i+100, i+200, Black8);
  // Draw a string.
  drawString(50, 250, "a string", Red8);
  // Draw another string but with font size=18 and in italic.
  drawString(40, 270, "another string", Magenta8, 18, 0, true);   
  // ... font size=24, rotation angle=-10, bold
  drawString(30,300,"yet another string",Black8,24,-10,
             false,true);
  // Draw a polygon with the following points.
  int px[] = { 201, 200, 260, 240 };
  int py[] = { 301, 350, 330, 280 };
  fillPoly(px, py, 4, Blue8);
  // Draw another polygon.
  Point2i P[]={
    Point2i(100, 100),
    Point2i(100, 150),
    Point2i(150, 120)
  };
  drawPoly(P, 3, Red8, 3);  // ... with a red pen with 3-pixel thickness.

  // Draw a blue arrow from (100,450) to (200,450).
  drawArrow(100, 470, 200, 450, Blue8);
  // Draw a red arrow with the a 30x10 pixels with style 1.
  // TODO: improve this API.
  drawArrow(300, 470, 200, 450, Red8, 30, 10, 1);
  drawArrow(200, 450, 250, 400, Black8, 20, 20, 2);
  // Draw another arrow with tip: (angle,length)=(35,8) , style=0, width=2.
  // TODO: improve this **horrible** legacy API.
  drawArrow(200, 450, 150, 400, Green8, 35., 8., 0, 2); 

  click();
  closeWindow(W);

  return 0;
}
