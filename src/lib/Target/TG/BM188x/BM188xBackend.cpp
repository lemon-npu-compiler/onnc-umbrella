//===- BM1880Backend.cpp --------------------------------------------------===//
//
//                             The ONNC Project
//
// See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "BM188xBackend.h"
#include "BM188x/BM188xTargetMemInfo.h"
#include "BM188x/BM188xTargetTransformInfo.h"
#include "BM188xCodeEmitter.h"
#include "BM188xFuseOptimizer.h"
#include "BM188xISelLowering.h"
#include <google/protobuf/text_format.h>

using namespace onnc;

// BM1880
BM1880Backend::BM1880Backend(const TargetOptions &pOptions)
    : TGBackend(new BM188xFuseOptimizer(this), new BM188xISelLowering(this),
                new BM188xCodeEmitter(this), pOptions)
{
  m_pMemInfo = new BM188xTargetMemInfo(this);
  m_pTTI = new BM188xTargetTransformInfo(this);
}

void BM1880Backend::addTensorSel(PassManager &pPM)
{
  pPM.add(createPrepareCtablePass(this));
  TGBackend::addTensorSel(pPM);
  pPM.add(createQuantizePass(this));
  pPM.add(createUpdateCtablePass(this));
  return;
}

bool BM1880Backend::isNativeTensorType(::onnx::TensorProto_DataType pType)
{
  switch (pType) {
  case ::onnx::TensorProto_DataType_INT8:
  case ::onnx::TensorProto_DataType_INT16:
    return true;
  default:
    return false;
  }
}

void BM1880Backend::setCtableProto(const std::string &pTextString)
{
  ::google::protobuf::TextFormat::ParseFromString(pTextString,
                                                  &m_NetCtableParam);
}

tg::bm1880::LayerCalibrationParameter *
BM1880Backend::getMutableLayerCtable(const std::string &pName)
{
  for (int i = 0; i < m_NetCtableParam.layer_size(); i++) {
    tg::bm1880::LayerCalibrationParameter *layer =
        m_NetCtableParam.mutable_layer(i);
    if (layer->name() == pName) {
      return layer;
    }
  }
  return nullptr;
}

const tg::bm1880::LayerCalibrationParameter *
BM1880Backend::getLayerCtable(const std::string &pName)
{
  for (int i = 0; i < m_NetCtableParam.layer_size(); i++) {
    const tg::bm1880::LayerCalibrationParameter &layer =
        m_NetCtableParam.layer(i);
    if (layer.name() == pName) {
      return &layer;
    }
  }
  return nullptr;
}
