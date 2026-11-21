#pragma once

namespace egomotion::cli { class IInvocationObserver; }
int egomotionApplication(int argc, char** argv, egomotion::cli::IInvocationObserver* observer = nullptr);
