// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

/*
 * This file must be adapted for every API individually!
 */

#ifndef DMITIGR_PGFE_DLL_HPP
#define DMITIGR_PGFE_DLL_HPP

#ifdef _WIN32
  #ifdef DMITIGR_PGFE_DLL_BUILDING
    #define DMITIGR_PGFE_API __declspec(dllexport)
  #else
    #if DMITIGR_PGFE_DLL
      #define DMITIGR_PGFE_API __declspec(dllimport)
    #else /* static or header-only library on Windows */
      #define DMITIGR_PGFE_API
    #endif
  #endif
#else /* Unix */
  #define DMITIGR_PGFE_API
#endif

#endif // DMITIGR_PGFE_DLL_HPP
