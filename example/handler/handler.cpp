#include "handler.h"

void Handler(context::Context& ctx, model::Req& req, model::Rsp& rsp) {
    rsp.data.emplace_back(req.id);
    rsp.data.emplace_back(int(req.name.size()));
}