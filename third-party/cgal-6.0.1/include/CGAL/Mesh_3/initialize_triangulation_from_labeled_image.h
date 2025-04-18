// Copyright (c) 2015,2016 GeometryFactory
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/v6.0.1/Mesh_3/include/CGAL/Mesh_3/initialize_triangulation_from_labeled_image.h $
// $Id: include/CGAL/Mesh_3/initialize_triangulation_from_labeled_image.h 50cfbde3b84 $
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Laurent Rineau

#ifndef CGAL_MESH_3_INITIALIZE_TRIANGULATION_FROM_LABELED_IMAGE_H
#define CGAL_MESH_3_INITIALIZE_TRIANGULATION_FROM_LABELED_IMAGE_H

#include <CGAL/license/Mesh_3.h>

#include <CGAL/Mesh_3/search_for_connected_components_in_labeled_image.h>
#include <CGAL/Distance_3/Point_3_Triangle_3.h>
#include <CGAL/Labeled_mesh_domain_3.h>
#include <CGAL/make_mesh_3.h>

#include <CGAL/enum.h>
#include <CGAL/iterator.h>
#include <CGAL/point_generators_3.h>
#include <CGAL/Image_3.h>

#include <iostream>
#include <queue>

template <typename Point>
struct Get_point
{
  const double vx, vy, vz;
  const double tx, ty, tz;
  const std::size_t xdim, ydim, zdim;
  Get_point(const CGAL::Image_3* image)
    : vx(image->vx())
    , vy(image->vy())
    , vz(image->vz())
    , tx(image->tx())
    , ty(image->ty())
    , tz(image->tz())
    , xdim(image->xdim())
    , ydim(image->ydim())
    , zdim(image->zdim())
  {}

  Point operator()(const std::size_t i,
                   const std::size_t j,
                   const std::size_t k) const
  {
    double x = double(i) * vx + tx;
    double y = double(j) * vy + ty;
    double z = double(k) * vz + tz;

    if (i == 0)              x += 1. / 6. * vx;
    else if (i == xdim - 1)  x -= 1. / 6. * vx;
    if (j == 0)              y += 1. / 6. * vy;
    else if (j == ydim - 1)  y -= 1. / 6. * vy;
    if (k == 0)              z += 1. / 6. * vz;
    else if (k == zdim - 1)  z -= 1. / 6. * vz;

    return Point(x, y, z);
  }
};
template<class C3T3, class MeshDomain, class MeshCriteria>
void init_tr_from_labeled_image_call_init_features(C3T3&,
                                                   const MeshDomain&,
                                                   const MeshCriteria&,
                                                   CGAL::Tag_false)
{
}
template<class C3T3, class MeshDomain, class MeshCriteria>
void init_tr_from_labeled_image_call_init_features(C3T3& c3t3,
                                                   const MeshDomain& domain,
                                                   const MeshCriteria& criteria,
                                                   CGAL::Tag_true)
{
  CGAL::Mesh_3::internal::init_c3t3_with_features(c3t3,
                                                  domain,
                                                  criteria);
  std::cout << c3t3.triangulation().number_of_vertices()
            << " initial points on 1D-features" << std::endl;
}

template<class C3T3, class MeshDomain, class MeshCriteria,
         typename Image_word_type,
         typename TransformOperator = CGAL::Identity<Image_word_type> >
void initialize_triangulation_from_labeled_image(C3T3& c3t3,
      const MeshDomain&   domain,
      const CGAL::Image_3& image,
      const MeshCriteria& criteria,
      Image_word_type,
      bool protect_features = false,
      TransformOperator transform = CGAL::Identity<Image_word_type>())
{
  typedef typename C3T3::Triangulation       Tr;
  typedef typename Tr::Geom_traits           GT;
  typedef typename GT::FT                    FT;
  typedef typename Tr::Weighted_point        Weighted_point;
  typedef typename Tr::Bare_point            Bare_point;
  typedef typename Tr::Segment               Segment_3;
  typedef typename Tr::Vertex_handle         Vertex_handle;
  typedef typename Tr::Cell_handle           Cell_handle;

  typedef typename GT::Vector_3              Vector_3;

  typedef MeshDomain                         Mesh_domain;

  Tr& tr = c3t3.triangulation();

  typename GT::Compare_weighted_squared_radius_3 cwsr =
    tr.geom_traits().compare_weighted_squared_radius_3_object();
  typename GT::Construct_point_3 cp =
    tr.geom_traits().construct_point_3_object();
  typename GT::Construct_weighted_point_3 cwp =
    tr.geom_traits().construct_weighted_point_3_object();

  if(protect_features) {
    init_tr_from_labeled_image_call_init_features
      (c3t3, domain, criteria,
       CGAL::internal::Has_features<Mesh_domain>());
  }

  const double max_v = (std::max)((std::max)(image.vx(),
                                             image.vy()),
                                  image.vz());

  struct Seed {
    std::size_t i, j, k;
    std::size_t radius;
  };
  using Seeds = std::vector<Seed>;
  using Subdomain = typename Mesh_domain::Subdomain;

  Seeds seeds;
  Get_point<Bare_point> get_point(&image);
  std::cout << "Searching for connected components..." << std::endl;
  search_for_connected_components_in_labeled_image(image,
                                                   std::back_inserter(seeds),
                                                   CGAL::Emptyset_iterator(),
                                                   transform,
                                                   Image_word_type());
  std::cout << "  " << seeds.size() << " components were found." << std::endl;
  std::cout << "Construct initial points..." << std::endl;
  for(const Seed seed : seeds)
  {
    const Bare_point seed_point = get_point(seed.i, seed.j, seed.k);
    Cell_handle seed_cell = tr.locate(cwp(seed_point));

    const Subdomain seed_label
      = domain.is_in_domain_object()(seed_point);
    const Subdomain seed_cell_label
      = (   tr.dimension() < 3
         || seed_cell == Cell_handle()
         || tr.is_infinite(seed_cell))
        ? Subdomain()  //seed_point is OUTSIDE_AFFINE_HULL
        : domain.is_in_domain_object()(
            seed_cell->weighted_circumcenter(tr.geom_traits()));

    if ( seed_label != std::nullopt
      && seed_cell_label != std::nullopt
      && *seed_label == *seed_cell_label)
        continue; //this means the connected component has already been initialized

    const double radius = double(seed.radius + 1)* max_v;
    CGAL::Random_points_on_sphere_3<Bare_point> points_on_sphere_3(radius);
    typename Mesh_domain::Construct_intersection construct_intersection =
      domain.construct_intersection_object();

    std::vector<Vector_3> directions;
    if(seed.radius < 2) {
      // shoot in six directions
      directions.push_back(Vector_3(-radius, 0, 0));
      directions.push_back(Vector_3(+radius, 0, 0));
      directions.push_back(Vector_3(0, -radius, 0));
      directions.push_back(Vector_3(0, +radius, 0));
      directions.push_back(Vector_3(0, 0, -radius));
      directions.push_back(Vector_3(0, 0, +radius));
    } else {
      for(int i = 0; i < 20; ++i)
      {
        // shoot 20 random directions
        directions.push_back(*points_on_sphere_3++ - CGAL::ORIGIN);
      }
    }

    for(const Vector_3& v : directions)
    {
      const Bare_point test = seed_point + v;

      const typename Mesh_domain::Intersection intersect =
        construct_intersection(Segment_3(seed_point, test));
      if (std::get<2>(intersect) != 0)
      {
        const Bare_point& bpi = std::get<0>(intersect);
        Weighted_point pi = cwp(bpi);

        // This would cause trouble to optimizers
        // check pi will not be hidden
        typename Tr::Locate_type lt;
        int li, lj;
        Cell_handle pi_cell = tr.locate(pi, lt, li, lj);
        if(lt != Tr::OUTSIDE_AFFINE_HULL) {
          switch (tr.dimension())
          { //skip dimension 0
          case 1:
            if (tr.side_of_power_segment(pi_cell, pi, true) != CGAL::ON_BOUNDED_SIDE)
              continue;
            break;
          case 2:
            if (tr.side_of_power_circle(pi_cell, 3, pi, true) != CGAL::ON_BOUNDED_SIDE)
              continue;
            break;
          case 3:
            if (tr.side_of_power_sphere(pi_cell, pi, true) != CGAL::ON_BOUNDED_SIDE)
              continue;
          }
        }

        //check pi is not inside a protecting ball
        std::vector<Vertex_handle> conflict_vertices;
        if (tr.dimension() == 3)
        {
          tr.vertices_on_conflict_zone_boundary(pi, pi_cell
            , std::back_inserter(conflict_vertices));
        }
        else
        {
          for (typename Tr::Finite_vertices_iterator vit = tr.finite_vertices_begin();
               vit != tr.finite_vertices_end(); ++vit)
          {
            const Weighted_point& wp = tr.point(vit);
            if (cwsr(wp, FT(0)) == CGAL::SMALLER) // 0 < wp's weight
              conflict_vertices.push_back(vit);
          }
        }

        bool pi_inside_protecting_sphere = false;
        for(Vertex_handle cv : conflict_vertices)
        {
          if(tr.is_infinite(cv))
            continue;

          const Weighted_point& cv_wp = tr.point(cv);
          if (cwsr(cv_wp, FT(0)) == CGAL::EQUAL) // 0 == wp's weight
            continue;

          // if the (squared) distance between bpi and cv is smaller or equal than cv's weight
          if (cwsr(cv_wp, - tr.min_squared_distance(bpi, cp(cv_wp))) != CGAL::LARGER)
          {
            pi_inside_protecting_sphere = true;
            break;
          }
        }
        if (pi_inside_protecting_sphere)
          continue;
        const typename Mesh_domain::Index index = std::get<1>(intersect);

        /// The following lines show how to insert initial points in the
        /// `c3t3` object. [insert initial points]
        Vertex_handle v = tr.insert(pi);

        // `v` could be null if `pi` is hidden by other vertices of `tr`.
        CGAL_assertion(v != Vertex_handle());

        c3t3.set_dimension(v, 2); // by construction, points are on surface
        c3t3.set_index(v, index);
        /// [insert initial points]
      }
      // else
      // {
      //   std::cerr <<
      //     boost::format("Error. Segment (%1%, %2%) does not intersect the surface!\n")
      //     % it->first % test;
      // }
    }
  }
  std::cout << "  " << tr.number_of_vertices() << " initial points." << std::endl;
  if ( c3t3.triangulation().dimension() != 3 )
  {
    std::cout << "  not enough points: triangulation.dimension() == "
              << c3t3.triangulation().dimension() << std::endl;
    CGAL::Mesh_3::internal::init_c3t3(c3t3, domain, criteria, 20);
    std::cout << "  -> " << tr.number_of_vertices() << " initial points." << std::endl;
  }
}

#endif // CGAL_MESH_3_INITIALIZE_TRIANGULATION_FROM_LABELED_IMAGE_H
