#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>

#include <CGAL/_test_cls_periodic_3_alpha_shape_3.h>

#include <CGAL/Alpha_shape_3.h>
#include <CGAL/Alpha_shape_cell_base_3.h>
#include <CGAL/Alpha_shape_vertex_base_3.h>
#include <CGAL/Periodic_3_Delaunay_triangulation_3.h>
#include <CGAL/Periodic_3_Delaunay_triangulation_traits_3.h>

#include <CGAL/Timer.h>

// Inexact construction types
typedef CGAL::Exact_predicates_inexact_constructions_kernel Gt;
typedef CGAL::Periodic_3_Delaunay_triangulation_traits_3<Gt> K;

// Vertex type
typedef CGAL::Periodic_3_triangulation_ds_vertex_base_3<> DsVb;
typedef CGAL::Triangulation_vertex_base_3<K,DsVb> Vb;
typedef CGAL::Alpha_shape_vertex_base_3<K,Vb> AsVb;
// Cell type
typedef CGAL::Periodic_3_triangulation_ds_cell_base_3<> DsCb;
typedef CGAL::Triangulation_cell_base_3<K,DsCb> Cb;
typedef CGAL::Alpha_shape_cell_base_3<K,Cb> AsCb;

typedef CGAL::Triangulation_data_structure_3<AsVb,AsCb> Tds;
typedef CGAL::Periodic_3_Delaunay_triangulation_3<K,Tds> P3DT3;
typedef CGAL::Alpha_shape_3<P3DT3>  Alpha_shape_3;


//Exact construction types
typedef CGAL::Exact_predicates_exact_constructions_kernel EGt;
typedef CGAL::Periodic_3_Delaunay_triangulation_traits_3<Gt> EK;

// Vertex type
typedef CGAL::Triangulation_vertex_base_3<EK,DsVb> EVb;
typedef CGAL::Alpha_shape_vertex_base_3<EK,EVb> EAsVb;
// Cell type
typedef CGAL::Triangulation_cell_base_3<EK,DsCb> ECb;
typedef CGAL::Alpha_shape_cell_base_3<EK,ECb> EAsCb;

typedef CGAL::Triangulation_data_structure_3<EAsVb,EAsCb> ETds;
typedef CGAL::Periodic_3_Delaunay_triangulation_3<EK,ETds> EP3DT3;
typedef CGAL::Alpha_shape_3<EP3DT3>  EAlpha_shape_3;

int main(int, char**)
{
  CGAL::Timer timer;
  timer.start();
  _test_cls_alpha_shape_3<Alpha_shape_3>();
  _test_cls_alpha_shape_3_exact<EAlpha_shape_3>();

  std::cerr << timer.time() << " sec." << std::endl;
  return 0;
}

// MipsPro prefers this after the other instantiations...
// Explicit instantiation of the whole class :
template class CGAL::Alpha_shape_3<P3DT3>;

