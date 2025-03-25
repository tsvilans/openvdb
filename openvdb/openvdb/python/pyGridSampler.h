// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#ifndef OPENVDB_PYGRIDSAMPLER_HAS_BEEN_INCLUDED
#define OPENVDB_PYGRIDSAMPLER_HAS_BEEN_INCLUDED

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/function.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/tuple.h>
#include <nanobind/ndarray.h>
#include <nanobind/stl/optional.h>

#include <openvdb/openvdb.h>
#include <openvdb/tools/Interpolation.h>
#include "pyutil.h"

namespace pyGridSampler {

    namespace nb = nanobind;
    using namespace openvdb::OPENVDB_VERSION_NAME;


    //@{
    /// Type traits for grid samplers
    template<typename _GridT, typename _SamplerT>
    struct GridSamplerTraitsBase
    {
        using GridT = _GridT;
        using SamplerT = _SamplerT;

        using NonConstGridT = GridT;
        using GridPtrT = typename NonConstGridT::Ptr;
        using ValueT = typename GridT::ValueType;

        static const bool IsConst = false;

        static const char* name();// { return "Sampler"; }
    };

    template<typename GridType, class SamplerType>
    struct GridSamplerTraits : public GridSamplerTraitsBase<GridType, SamplerType>
    {
    };

    /// Map a grid type to a traits class that derives from GridTraitsBase
    /// and that defines a name() method.
#define GRIDSAMPLER_TRAITS(_typ, _name) \
    template<typename GridType> struct GridSamplerTraits<GridType, _typ>: public GridSamplerTraitsBase<GridType, _typ> { \
        static const char* name() { return _name; } \
    }

    GRIDSAMPLER_TRAITS(openvdb::tools::PointSampler, "PointSampler");
    GRIDSAMPLER_TRAITS(openvdb::tools::BoxSampler, "BoxSampler");
    GRIDSAMPLER_TRAITS(openvdb::tools::QuadraticSampler, "QuadraticSampler");


    //@}


    ////////////////////////////////////////


    /// @brief GridSampler wrapper class that also stores a grid pointer,
    /// so that the grid doesn't get deleted as long as the accessor is live
    ///
    /// @internal This class is directly adapted from ValueAccessorWrap
    template<typename _GridType, typename _SamplerType>
    class GridSamplerWrap
    {
    public:
        using Traits = GridSamplerTraits<_GridType, _SamplerType>;
        using ValueType = typename Traits::ValueT;
        using GridType = typename Traits::NonConstGridT;
        using GridPtrType = typename Traits::GridPtrT;

        GridSamplerWrap(const GridPtrType& grid) : mGrid(grid), mSampler(*grid) {}

        GridSamplerWrap copy() const { return *this; }

        GridPtrType parent() const { return mGrid; }

        ValueType sampleVoxel(const Coord& ijk)
        {
            return mSampler.sampleVoxel(ijk.x, ijk.y, ijk.z);
        }

        ValueType isSample(const Coord& ijk)
        {
            return mSampler.isSample(ijk);
        }

        //ValueType isSample(const Vec3d& ijk)
        //{
        //    return mSampler.isSample(ijk);
        //}

        ValueType wsSample(const Vec3d& wsPoint)
        {
            return mSampler.wsSample(wsPoint);
        }

        /// @brief Define a Python wrapper class for this C++ class.
        static void wrap(nb::module_ m)
        {
            const std::string
                pyGridTypeName = pyutil::GridTraits<GridType>::name(),
                pyValueTypeName = openvdb::typeNameAsString<typename GridType::ValueType>(),
                pyGridSamplerTypeName = Traits::name();

            nb::class_<GridSamplerWrap>(m,
                (pyGridTypeName + pyGridSamplerTypeName).c_str(), //pybind11 requires a unique class name for each template instantiation
                ("Class that provides the interface for continuous sampling of \nvalues in a "
                    + pyGridTypeName).c_str())
                .def(nb::init<const GridPtrType&>(), nb::arg("grid"),
                    "Initialize with a grid to be sampled.")
                .def("copy", &GridSamplerWrap::copy,
                    ("copy() -> " + pyGridSamplerTypeName + "\n\n"
                        "Return a copy of this grid sampler.").c_str())
                .def_prop_ro("parent", &GridSamplerWrap::parent,
                    ("this grid sampler's parent " + pyGridTypeName).c_str())

                //
                // Voxel access
                //
                .def("isSample", &GridSamplerWrap::isSample,
                    nb::arg("ijk"),
                    ("isSample(ijk) -> " + pyValueTypeName + "\n\n"
                        "Return the index-space value at coordinates (i, j, k).").c_str())
                .def("wsSample", &GridSamplerWrap::wsSample,
                    nb::arg("xyz"),
                    ("wsSample(xyz) -> " + pyValueTypeName + "\n\n"
                        "Return the world-space value at coordinates (x, y, z).").c_str())

                ; // nb::class_<GridSampler>
        }

    private:
        const GridPtrType mGrid;
        openvdb::tools::GridSampler<GridType, _SamplerType> mSampler;
    }; // class GridSamplerWrap

} // namespace pyGridSampler

#endif // OPENVDB_PYGRIDSAMPLER_HAS_BEEN_INCLUDED
