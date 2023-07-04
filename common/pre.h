#pragma once

#include <string>

enum class ERequests {
    ERViewBook,
    ERMakeOrder,
    ERViewUserInfo,
    ERCancelOrder,
    ERExit,
    ERAuth,
    ERReg,
};

enum class EFields {
    EFUserId,
    EFOrderId,
    EFSide,
    EFQuantity,
    EFPrice,
    EFReqType,
    EFMessage
};

enum class ESide {
    ESell,
    EBuy
};
