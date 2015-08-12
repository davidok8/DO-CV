#include <DO/Sara/Graphics.hpp>
#include <DO/Sara/ImageProcessing.hpp>


using namespace std;
using namespace DO::Sara;


void create_ellipse()
{
  create_window(300, 300);
  fill_rect(0, 0, 100, 100, Black8);
  save_screen(active_window(), src_path("ellipse.png"));
  get_key();
  close_window();
}

Image<float> warp(const Image<float>& I, const Matrix2f& T)
{
  Image<float> warped_image(300, 300);
  warped_image.array().fill(0.f);
  for (int y = 0; y < warped_image.height(); ++y)
  {
    for (int x = 0; x < warped_image.width(); ++x)
    {
      auto p = Vector2d{ (T * Point2f{ x, y }).cast<double>() };
      if ( p.x() < 0 || p.x() >= I.width()-1  ||
           p.y() < 0 || p.y() >= I.height()-1 )
        continue;
      warped_image(x,y) = interpolate(I, p);
    }
  }
  return warped_image;
}


GRAPHICS_MAIN()
{
  Image<float> I;
  create_ellipse();
  if (!load(I, src_path("ellipse.png")))
    return -1;
  //I = color_rescale(dericheBlur(I, 50.f));
  create_window(I.width(), I.height());

  Matrix2f finalT;
  finalT.setIdentity();

  Image<float> old_I{ I };
  Image<float> diff{ I };

  const int iter = 1000;
  for (int i = 0; i < iter; ++i)
  {
    // Check the image.
    display(I);
    get_key();

    diff.array() = I.array() - old_I.array();
    diff = color_rescale(diff);
    display(diff);
    get_key();

    // Compute the second order moment matrix.
    Image<Matrix2f> M(I.compute<Gradient>().compute<SecondMomentMatrix>());
    Matrix2f Sigma;
    for (Image<Matrix2f>::iterator M_it = M.begin(); M_it != M.end(); ++M_it)
      Sigma += *M_it;
    Sigma /= Sigma.norm();


    // Get the SVD decomposition of the second order moment matrix.
    JacobiSVD<Matrix2f> svd(Sigma, ComputeFullU);
    Vector2f sv = svd.singularValues();
    Matrix2f U = svd.matrixU();

    // Get one inverse transform.
    Vector2f radii( sv.cwiseSqrt().cwiseInverse() );
    Matrix2f T( U*radii.asDiagonal()*U.transpose() );
    T *= 1.f/radii(1);

    // Check.
    float angle =  atan2(U(1,0), U(0,0));
    angle = angle/(2*M_PI)*360.f;
    float rmin = 1.f/sqrt(sv(1));
    float rmax = 1.f/sqrt(sv(0));

    print_stage("Iteration " + to_string(i));
    cout << "Sigma = " << endl << Sigma << endl << endl;
    cout << "U*S*U^T = " << endl <<  U*sv.asDiagonal()*U.transpose() << endl << endl;
    cout << "T = " << endl << T << endl << endl;
    cout << "radii = " << radii.transpose() << endl << endl;
    cout << "scaleFactor= " << sqrt(sv(0)/sv(1)) << endl << endl;

    cout << "singular values = " << sv.transpose() << endl;
    cout << "U = " << endl << U << endl;
    cout << "angle = " << angle << " degrees" << endl;
    cout << "rmin = " << rmin << " rmax = " << rmax << endl;
    cout << "ratio = " << rmax/rmin << endl;

    old_I = I;
    I = warp(I, T);
  }

  return 0;
}
