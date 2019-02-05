#include "IPC.hpp"
#include "Numpy.hpp"

#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

#include <iostream>


namespace bip = boost::interprocess;
namespace bp = boost::python;


template <typename T>
using ipc_allocator =
    bip::allocator<T, bip::managed_shared_memory::segment_manager>;

template <typename T>
using ipc_vector = bip::vector<T, ipc_allocator<T>>;


template <class T>
bp::list to_py_list(const ipc_vector<T>& v)
{
  auto l = bp::list{};
  for (const auto& c : v)
    l.append(c);
  return l;
}


namespace np = bp::numpy;


class IpcMedium
{
public:
  IpcMedium(const std::string& segment_name)
    : _segment{bip::open_only, segment_name.c_str()}
  {
  }

  np::ndarray tensor(const std::string& name)
  {
    const auto image_shape_name = name + "_shape";
    const auto image_data_name = name + "_data";

    auto image_shape =
        _segment.find<ipc_vector<int>>(image_shape_name.c_str()).first;
    auto image_data =
        _segment.find<ipc_vector<float>>(image_data_name.c_str()).first;

    const auto shape = bp::tuple(to_py_list(*image_shape));
    const auto strides =
        bp::make_tuple(sizeof(float) * (*image_shape)[1], sizeof(float));

    return np::from_data(image_data->data(), np::dtype::get_builtin<float>(),
                         shape, strides, bp::object());
  }

private:
  bip::managed_shared_memory _segment;
};


void expose_ipc()
{
  bp::class_<IpcMedium, boost::noncopyable>("IpcMedium",
                                            bp::init<const std::string&>())
      .def("tensor", &IpcMedium::tensor);
}
