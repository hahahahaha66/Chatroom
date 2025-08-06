#include "Poller.h"

Poller::Poller(EventLoop *Loop) : ownerLoop_(Loop) {}

Poller::~Poller() = default;

bool Poller::hasChannel(Channel *channel) const {
    auto it = channels_.find(channel->fd());
    return it != channels_.end() && it->second == channel;
}