// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_FIDO_BIO_ENROLLER_H_
#define DEVICE_FIDO_BIO_ENROLLER_H_

#include <vector>

#include "base/memory/weak_ptr.h"
#include "base/optional.h"
#include "base/sequence_checker.h"
#include "device/fido/bio/enrollment.h"
#include "device/fido/pin.h"

namespace device {

class FidoAuthenticator;

// Handles enrolling fingerprints in an authenticator.
class BioEnroller {
 public:
  class Delegate {
   public:
    // Called when the authenticator reports a sample has been collected. Does
    // not indicate success.
    virtual void OnSampleCollected(BioEnrollmentSampleStatus status,
                                   int samples_remaining) = 0;
    // Called when the enrollment has completed. |template_id| may be
    // base::nullopt if the enrollment has been cancelled.
    virtual void OnEnrollmentDone(
        base::Optional<std::vector<uint8_t>> template_id) = 0;
    virtual void OnEnrollmentError(CtapDeviceResponseCode status) = 0;
  };

  BioEnroller(Delegate* delegate,
              FidoAuthenticator* authenticator,
              pin::TokenResponse token);
  ~BioEnroller();
  BioEnroller(const BioEnroller&) = delete;
  BioEnroller(BioEnroller&&) = delete;

  // Attempts to cancel an in progress enrollment. Does nothing if the request
  // is already completed (i.e. |OnEnrollmentDone| has been called already).
  // Otherwise, guaranteed to call |OnEnrollmentDone|.
  void Cancel();

  pin::TokenResponse token() { return token_; }

 private:
  enum State {
    kInProgress,
    kCancelled,
    kDone,
  };

  void FinishWithError(CtapDeviceResponseCode status);
  void FinishSuccessfully(base::Optional<std::vector<uint8_t>> template_id);

  void OnEnrollResponse(CtapDeviceResponseCode status,
                        base::Optional<BioEnrollmentResponse> response);
  void OnEnrollCancelled(CtapDeviceResponseCode status,
                         base::Optional<BioEnrollmentResponse> response);

  State state_ = State::kInProgress;
  Delegate* delegate_;
  FidoAuthenticator* authenticator_;
  pin::TokenResponse token_;
  base::Optional<std::vector<uint8_t>> template_id_;

  SEQUENCE_CHECKER(my_sequence_checker_);
  base::WeakPtrFactory<BioEnroller> weak_factory_{this};
};

}  // namespace device

#endif  // DEVICE_FIDO_BIO_ENROLLER_H_
