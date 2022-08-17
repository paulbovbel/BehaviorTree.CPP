#ifndef BT_FLATBUFFER_HELPER_H
#define BT_FLATBUFFER_HELPER_H

#include "behaviortree_cpp_v3/bt_factory.h"
#include "BT_logger_generated.h"

namespace BT
{

typedef std::array<uint8_t, 12> SerializedTransition;

Serialization::NodeType convertToFlatbuffers(BT::NodeType type);

Serialization::NodeStatus convertToFlatbuffers(BT::NodeStatus type);

Serialization::PortDirection convertToFlatbuffers(BT::PortDirection direction);

void CreateFlatbuffersBehaviorTree(flatbuffers::FlatBufferBuilder& builder,
                                   const BT::Tree& tree);

using FullTreeStatus = std::unordered_map<uint32_t, BT::NodeStatus>;

void CreateFlatbuffersTreeStatus(flatbuffers::FlatBufferBuilder& builder,
                                 Duration timestamp,
                                 const FullTreeStatus& tree_status);

/** Serialize manually the informations about state transition
 * No flatbuffer serialization here
 */
SerializedTransition SerializeTransition(uint16_t UID,
                                         Duration timestamp,
                                         NodeStatus prev_status,
                                         NodeStatus status);
}   // end namespace

#endif   // BT_FLATBUFFER_HELPER_H
