Requirements
============

  * The user must be allowed to create throw-away grids for reading

  * It must be avoided that there are several fragments with the exact same shape

  * It must be avoided that there are several grids with the exact same axis and subgrid structure

  * => deduplication must happen on two distinct levels: the fragment level and the grid level

  * It is not possible to reference count grids or fragments, as these objects contain a reference to their owning dataset, and thus must not survive its destruction
      * => deduplication must rely on proxy objects that reference the already existing objects

  * User code can hold pointers to subgrids
      * => grids cannot be replaced with proxy objects, they must be turned into proxy objects
      * => grids that are turned into proxy objects must retain their subgrid structure to ensure proper destruction

  * A fragment cannot be matched with a subgrid, and a subgrid cannot be matched with another subgrid that differs in one of the axes or subgrids
      * => deduplication cannot happen while the grid is still in fixed axes state
      * => esdm_read_grid() and esdm_write_grid() should be defined to put a grid into fixed structure state, allowing deduplication on their first call



Implementation
==============

  * Fragments are managed in a centralized container (hashtable keyed by their extends), and referenced by the grids.

      * A count of referencing grids can be added if necessary.
        This is not a general ref count, the dataset still owns the fragments and destructs them in its destructor.
        The grid count would allow the dataset to delete a fragment when its containing grids get deleted.

      * The fragment references in the grids remain plain pointers, but the grids loose their ownership over the fragments.

      * The MPI code that handles grids must be expanded to also communicate fragment information explicitly.

      * As a side effect, this also allows the dataset to unload any fragments to prevent run-away memory consumption.


  * Grids contain a delegate pointer.

      * All grid methods must first resolve any delegate chain.

      * The delegate pointer is set when the grid is first touched with an I/O or MPI call and detected to be structurally identical to an existing grid.

      * When the delegate pointer is set, the delegate pointers of all subgrids are also set.

      * There are two possible implementations for managing grid proxy objects:
          * The proxy grid's structure data remains valid (axes and cell matrix), the proxy's destructor recursively destructs its subgrid proxies.
          * All existing grids are managed via a flat list of grids within the dataset, and grids that become proxies get their axis and matrix data deleted immediately.
