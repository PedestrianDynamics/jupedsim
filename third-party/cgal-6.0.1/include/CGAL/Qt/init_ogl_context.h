// Copyright (c) 2021  GeometryFactory Sarl (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/v6.0.1/GraphicsView/include/CGAL/Qt/init_ogl_context.h $
// $Id: include/CGAL/Qt/init_ogl_context.h 50cfbde3b84 $
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Maxime Gimeno

#ifndef CGAL_QT_CONTEXT_INITIALIZATION_H
#define CGAL_QT_CONTEXT_INITIALIZATION_H

#include <CGAL/license/GraphicsView.h>


#include <QSurfaceFormat>
#include <QCoreApplication>
namespace CGAL
{
namespace Qt
{
inline void init_ogl_context(int major, int minor) {
  QSurfaceFormat fmt;
#ifdef Q_OS_MAC
  if(major == 4)
  {
    fmt.setVersion(4, 1);
  }
  else
  {
     fmt.setVersion(major, minor);
  }
#else
  fmt.setVersion(major, minor);
#endif
  fmt.setRenderableType(QSurfaceFormat::OpenGL);
  fmt.setProfile(QSurfaceFormat::CoreProfile);
  fmt.setOption(QSurfaceFormat::DebugContext);
  QSurfaceFormat::setDefaultFormat(fmt);

  //for windows
  QCoreApplication::setAttribute(::Qt::AA_UseDesktopOpenGL);

  //We set the locale to avoid any trouble with VTK
  setlocale(LC_ALL, "C");
}

} //end Qt
} //end CGAL
#endif // CGAL_QT_CONTEXT_INITIALIZATION_H
