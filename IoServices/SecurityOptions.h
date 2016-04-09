//
//  SecurityOptions.h
//  HelloAsio
//
//  Created by Michael Jones on 9/04/2016.
//  Copyright © 2016 Michael Jones. All rights reserved.
//

#ifndef SecurityOptions_h
#define SecurityOptions_h

#if defined(__GNUC__)
/* The classes below are exported */
#pragma GCC visibility push(default)
#endif

#include <string>
#include <functional>

namespace AsyncIo {
    struct SecurityOptions {
        std::string CertificateFilename{};
        std::string PrivKeyFilename{};
        std::string DHExchangeFilename{};
        std::function<std::string()> GetPasswordCallback{};
    };
}

#endif /* SecurityOptions_h */
