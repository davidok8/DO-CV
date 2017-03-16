#define BOOST_TEST_NO_MAIN
#define BOOST_TEST_MODULE "Features/Drawing Functions"

#include <boost/test/unit_test.hpp>

#include <DO/Sara/Features/Draw.hpp>

#include <DO/Sara/Graphics.hpp>


using namespace std;
using namespace DO::Sara;


BOOST_AUTO_TEST_SUITE(TestOERegionDrawing)

BOOST_AUTO_TEST_CASE(test_draw_method)
{
  create_window(300, 300);
  auto f = OERegion{Point2f{300 / 2.f, 300 / 2.f}, 1.f};
  f.draw(Red8);
  close_window();
}

BOOST_AUTO_TEST_CASE(test_draw_oe_regions)
{
  create_window(300, 300);
  auto features = vector<OERegion>{
    OERegion{ Point2f{ 300 / 2.f, 300 / 2.f }, 1.f },
    OERegion{ Point2f{ 200.f, 300 / 2.f }, 1.f }
  };
  draw_oe_regions(features, Red8);
  close_window();
}

BOOST_AUTO_TEST_SUITE_END()

int worker_thread(int argc, char **argv)
{
  return boost::unit_test::unit_test_main([]() { return true; }, argc, argv);
}

int main(int argc, char **argv)
{
  GraphicsApplication gui_app(argc, argv);
  gui_app.register_user_main(worker_thread);
  return gui_app.exec();
}
