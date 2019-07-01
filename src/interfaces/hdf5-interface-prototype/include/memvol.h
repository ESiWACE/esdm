// This file is part of h5-memvol.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with h5-memvol.  If not, see <http://www.gnu.org/licenses/>.

/**
 * \file
 * \brief Main interfaces for the memvol HDF5 plugin
 * \author Julian Kunkel <juliankunkel@googlemail.com>

\startuml{memvol-components.png}
  title Components of memvol
  skinparam shadowing false

  folder "Core in src/" {

    frame "libscil" {
        'component X #PowderBlue
        interface "scil.h" #Orange
        component "scil-algo-chooser" #Wheat

        'note left of X
        'end note
        '[Thread] ..> [SIOX-LL] : use
      }

    folder "tools" {
      artifact [scil-benchmark]
    }

    folder "pattern"{
      frame "libscil-patterns"{
        interface "scil-patterns.h" #Purple
      }
    }
  }

  actor admin
  admin --> [scil-benchmark] : runs
\enduml
 */

#ifndef H5_MEMVOL_HEADER__
#define H5_MEMVOL_HEADER__

#include <hdf5.h>
#include <stdio.h>

/*
  Returns the volume id.
*/
hid_t H5VL_memvol_init();
int H5VL_memvol_finalize();

#endif
