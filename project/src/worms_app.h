// worms - ReXGlue Recompiled Project
//
// Customize your app by overriding virtual hooks from rex::ReXApp.

#pragma once

#include <rex/rex_app.h>

#include <cstdlib>
#include <fstream>

class WormsApp : public rex::ReXApp {
 public:
  using rex::ReXApp::ReXApp;

  static std::unique_ptr<rex::ui::WindowedApp> Create(
      rex::ui::WindowedAppContext& ctx) {
    return std::unique_ptr<WormsApp>(new WormsApp(ctx, "worms",
        PPCImageConfig));
  }

  // ponytail: one-shot bring-up scaffold. Dump the runtime-decompressed image
  // straight out of mapped guest memory (extract_pe.py can't handle this title's
  // LZX variant) so find_missing_vtable_funcs.py can batch-register vtable/thunk
  // targets. Enabled only when REX_DUMP_IMAGE is set; remove after the sweep.
  void OnPostLoadXexImage() override {
    if (const char* path = std::getenv("REX_DUMP_IMAGE")) {
      constexpr uint32_t kBase = 0x82000000u;   // triaged image base
      constexpr uint32_t kSize = 0x011E0000u;   // triaged image size
      const uint8_t* img = rex::Runtime::instance()->virtual_membase() + kBase;
      std::ofstream(path, std::ios::binary)
          .write(reinterpret_cast<const char*>(img), kSize);
    }
  }

  // Override virtual hooks for customization:
  // void OnPostInitLogging() override {}
  // void OnPreSetup(rex::RuntimeConfig& config) override {}
  // void OnPostSetup() override {}
  // void OnCreateDialogs(rex::ui::ImGuiDrawer* drawer) override {}
  // void OnShutdown() override {}
  // void OnConfigurePaths(rex::PathConfig& paths) override {}
};
