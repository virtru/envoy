#include "common/network/socket_option_factory.h"

#include "common/common/fmt.h"
#include "common/network/addr_family_aware_socket_option_impl.h"
#include "common/network/socket_option_impl.h"

namespace Envoy {
namespace Network {

std::unique_ptr<Socket::Options>
SocketOptionFactory::buildTcpKeepaliveOptions(Network::TcpKeepaliveConfig keepalive_config) {
  std::unique_ptr<Socket::Options> options = std::make_unique<Socket::Options>();
  options->push_back(std::make_shared<Network::SocketOptionImpl>(
      envoy::api::v2::core::SocketOption::STATE_PREBIND, ENVOY_SOCKET_SO_KEEPALIVE, 1));

  if (keepalive_config.keepalive_probes_.has_value()) {
    options->push_back(std::make_shared<Network::SocketOptionImpl>(
        envoy::api::v2::core::SocketOption::STATE_PREBIND, ENVOY_SOCKET_TCP_KEEPCNT,
        keepalive_config.keepalive_probes_.value()));
  }
  if (keepalive_config.keepalive_interval_.has_value()) {
    options->push_back(std::make_shared<Network::SocketOptionImpl>(
        envoy::api::v2::core::SocketOption::STATE_PREBIND, ENVOY_SOCKET_TCP_KEEPINTVL,
        keepalive_config.keepalive_interval_.value()));
  }
  if (keepalive_config.keepalive_time_.has_value()) {
    options->push_back(std::make_shared<Network::SocketOptionImpl>(
        envoy::api::v2::core::SocketOption::STATE_PREBIND, ENVOY_SOCKET_TCP_KEEPIDLE,
        keepalive_config.keepalive_time_.value()));
  }
  return options;
}

std::unique_ptr<Socket::Options> SocketOptionFactory::buildIpFreebindOptions() {
  std::unique_ptr<Socket::Options> options = std::make_unique<Socket::Options>();
  options->push_back(std::make_shared<Network::AddrFamilyAwareSocketOptionImpl>(
      envoy::api::v2::core::SocketOption::STATE_PREBIND, ENVOY_SOCKET_IP_FREEBIND,
      ENVOY_SOCKET_IPV6_FREEBIND, 1));
  return options;
}

std::unique_ptr<Socket::Options> SocketOptionFactory::buildIpTransparentOptions() {
  std::unique_ptr<Socket::Options> options = std::make_unique<Socket::Options>();
  options->push_back(std::make_shared<Network::AddrFamilyAwareSocketOptionImpl>(
      envoy::api::v2::core::SocketOption::STATE_PREBIND, ENVOY_SOCKET_IP_TRANSPARENT,
      ENVOY_SOCKET_IPV6_TRANSPARENT, 1));
  options->push_back(std::make_shared<Network::AddrFamilyAwareSocketOptionImpl>(
      envoy::api::v2::core::SocketOption::STATE_BOUND, ENVOY_SOCKET_IP_TRANSPARENT,
      ENVOY_SOCKET_IPV6_TRANSPARENT, 1));
  return options;
}

std::unique_ptr<Socket::Options> SocketOptionFactory::buildSocketMarkOptions(uint32_t mark) {
  std::unique_ptr<Socket::Options> options = std::make_unique<Socket::Options>();
  // we need this to happen prior to binding or prior to connecting. In both cases, PREBIND will
  // fire.
  options->push_back(std::make_shared<Network::SocketOptionImpl>(
      envoy::api::v2::core::SocketOption::STATE_PREBIND, ENVOY_SOCKET_SO_MARK, mark));
  return options;
}

std::unique_ptr<Socket::Options> SocketOptionFactory::buildLiteralOptions(
    const Protobuf::RepeatedPtrField<envoy::api::v2::core::SocketOption>& socket_options) {
  auto options = std::make_unique<Socket::Options>();
  for (const auto& socket_option : socket_options) {
    std::string buf;
    int int_value;
    switch (socket_option.value_case()) {
    case envoy::api::v2::core::SocketOption::kIntValue:
      int_value = socket_option.int_value();
      buf.append(reinterpret_cast<char*>(&int_value), sizeof(int_value));
      break;
    case envoy::api::v2::core::SocketOption::kBufValue:
      buf.append(socket_option.buf_value());
      break;
    default:
      ENVOY_LOG(warn, "Socket option specified with no or unknown value: {}",
                socket_option.DebugString());
      continue;
    }
    options->emplace_back(std::make_shared<Network::SocketOptionImpl>(
        socket_option.state(),
        Network::SocketOptionName(
            socket_option.level(), socket_option.name(),
            fmt::format("{}/{}", socket_option.level(), socket_option.name())),
        buf));
  }
  return options;
}

std::unique_ptr<Socket::Options>
SocketOptionFactory::buildTcpFastOpenOptions(uint32_t queue_length) {
  std::unique_ptr<Socket::Options> options = std::make_unique<Socket::Options>();
  options->push_back(std::make_shared<Network::SocketOptionImpl>(
      envoy::api::v2::core::SocketOption::STATE_LISTENING, ENVOY_SOCKET_TCP_FASTOPEN,
      queue_length));
  return options;
}

std::unique_ptr<Socket::Options> SocketOptionFactory::buildIpPacketInfoOptions() {
  std::unique_ptr<Socket::Options> options = std::make_unique<Socket::Options>();
  options->push_back(std::make_shared<AddrFamilyAwareSocketOptionImpl>(
      envoy::api::v2::core::SocketOption::STATE_BOUND, ENVOY_RECV_IP_PKT_INFO,
      ENVOY_RECV_IPV6_PKT_INFO, 1));
  return options;
}

std::unique_ptr<Socket::Options> SocketOptionFactory::buildRxQueueOverFlowOptions() {
  std::unique_ptr<Socket::Options> options = std::make_unique<Socket::Options>();
#ifdef SO_RXQ_OVFL
  options->push_back(std::make_shared<Network::SocketOptionImpl>(
      envoy::api::v2::core::SocketOption::STATE_BOUND,
      ENVOY_MAKE_SOCKET_OPTION_NAME(SOL_SOCKET, SO_RXQ_OVFL), 1));
#endif
  return options;
}

} // namespace Network
} // namespace Envoy
