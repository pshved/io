/* Copyright 2018 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "kernels/dataset_ops.h"
#include "tensorflow/core/lib/io/buffered_inputstream.h"

namespace tensorflow {
namespace data {

class TextInput: public DataInput<io::BufferedInputStream> {
 public:
  Status ReadRecord(io::InputStreamInterface& s, IteratorContext* ctx, std::unique_ptr<io::BufferedInputStream>& state, int64* returned, std::vector<Tensor>* out_tensors) const override {
    if (state.get() == nullptr) {
      state.reset(new io::BufferedInputStream(&s, 4096));
    }
    string buffer;
    Status status = state.get()->ReadLine(&buffer);
    if (!(status.ok() || errors::IsOutOfRange(status))) {
      return status;
    }
    *returned = (status.ok()) ? 1 : 0;
    if (*returned == 1) {
      Tensor value_tensor(ctx->allocator({}), DT_STRING, {});
      value_tensor.scalar<string>()() = buffer;
      out_tensors->emplace_back(std::move(value_tensor));
    }
    return Status::OK();
  }
  Status FromStream(io::InputStreamInterface& s) override {
    // TODO: Read 4K buffer to detect BOM.
    //string header;
    //TF_RETURN_IF_ERROR(s.ReadNBytes(4096, &header));
    //for (size i = 0; i < header.size(); i++) {
    //  if (!isprint(header[i])) {
    //    return errors::InvalidArgument("text file contains character that is non printable at ", i);
    //  }
    //}
    return Status::OK();
  }
  void EncodeAttributes(VariantTensorData* data) const override {
  }
  bool DecodeAttributes(const VariantTensorData& data) override {
    return true;
  }
 protected:
};

REGISTER_UNARY_VARIANT_DECODE_FUNCTION(TextInput, "tensorflow::data::TextInput");

REGISTER_KERNEL_BUILDER(Name("TextInput").Device(DEVICE_CPU),
                        DataInputOp<TextInput>);
REGISTER_KERNEL_BUILDER(Name("TextDataset").Device(DEVICE_CPU),
                        InputDatasetOp<TextInput, io::BufferedInputStream>);
}  // namespace data
}  // namespace tensorflow
