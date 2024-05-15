// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#ifndef OPENVDB_PYGRIDSAMPLER_HAS_BEEN_INCLUDED
#define OPENVDB_PYGRIDSAMPLER_HAS_BEEN_INCLUDED

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <openvdb/openvdb.h>
#include <openvdb/tools/Interpolation.h>
#include "pyutil.h"

namespace pyGridSampler {

    namespace py = pybind11;
    using namespace openvdb::OPENVDB_VERSION_NAME;


    //@{
    /// Type traits for grid accessors
    template<typename _GridT, typename _SamplerT>
    struct GridSamplerTraitsBase
    {
        using GridT = _GridT;
        using SamplerT = _SamplerT;

        using NonConstGridT = GridT;
        using GridPtrT = typename NonConstGridT::Ptr;
        //using AccessorT = typename NonConstGridT::Accessor;
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


    /// @brief ValueAccessor wrapper class that also stores a grid pointer,
    /// so that the grid doesn't get deleted as long as the accessor is live
    ///
    /// @internal This class could have just been made to inherit from ValueAccessor,
    /// but the method wrappers allow for more Pythonic error messages.  For example,
    /// if we constructed the Python getValue() method directly from the corresponding
    /// ValueAccessor method, as follows,
    ///
    ///    .def("getValue", &Accessor::getValue, ...)
    ///
    /// then the conversion from a Python type to a Coord& would be done
    /// automatically.  But if the Python method were called with an object of
    /// a type that is not convertible to a Coord, then the TypeError message
    /// would say something like "TypeError: No registered converter was able to
    /// produce a C++ rvalue of type openvdb::math::Coord...".
    /// Handling the type conversion manually is more work, but it allows us to
    /// instead generate messages like "TypeError: expected tuple(int, int, int),
    /// found str as argument to FloatGridAccessor.getValue()".
    template<typename _GridType, typename _SamplerType>
    class GridSamplerWrap
    {
    public:
        using Traits = GridSamplerTraits<_GridType, _SamplerType>;
        //using GridSampler = typename Traits::AccessorT;
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
        static void wrap(py::module_ m)
        {
            const std::string
                pyGridTypeName = pyutil::GridTraits<GridType>::name(),
                pyValueTypeName = openvdb::typeNameAsString<typename GridType::ValueType>(),
                pyGridSamplerTypeName = Traits::name();

            py::class_<GridSamplerWrap>(m,
                (pyGridTypeName + pyGridSamplerTypeName).c_str(), //pybind11 requires a unique class name for each template instantiation
                ("Read/write access by (i, j, k) index coordinates to the voxels\nof a "
                    + pyGridTypeName).c_str())
                .def(py::init<const GridPtrType&>(), py::arg("grid"),
                    "Initialize with a grid to be sampled.")
                .def("copy", &GridSamplerWrap::copy,
                    ("copy() -> " + pyGridSamplerTypeName + "\n\n"
                        "Return a copy of this accessor.").c_str())
                .def_property_readonly("parent", &GridSamplerWrap::parent,
                    ("this accessor's parent " + pyGridTypeName).c_str())

                //
                // Voxel access
                //
                .def("isSample", &GridSamplerWrap::isSample,
                    py::arg("ijk"),
                    ("isSample(ijk) -> " + pyValueTypeName + "\n\n"
                        "Return the value of the voxel in index space at coordinates (i, j, k).").c_str())
                .def("wsSample", &GridSamplerWrap::wsSample,
                    py::arg("ijk"),
                    ("wsSample(ijk) -> " + pyValueTypeName + "\n\n"
                        "Return the value of the voxel in world space at coordinates (i, j, k).").c_str())

                ; // py::class_<ValueAccessor>
        }

    private:
        const GridPtrType mGrid;
        openvdb::tools::GridSampler<GridType, _SamplerType> mSampler;
    }; // class AccessorWrap

} // namespace pyAccessor

#endif // OPENVDB_PYGRIDSAMPLER_HAS_BEEN_INCLUDED
