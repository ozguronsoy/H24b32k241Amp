def process_dsp_lib():
    dsp_lib_path = os.path.join(FRAMEWORK_DIR, "Drivers", "CMSIS", "DSP", "Lib", "GCC")
    if not os.path.isdir(dsp_lib_path):
        dsp_lib_path = os.path.join(FRAMEWORK_DIR, "Drivers", "CMSIS", "Lib", "GCC")

          
    env.Append(
        CPPPATH=[
            os.path.join(FRAMEWORK_DIR, "Drivers", "CMSIS", "DSP", "Include"),
        ],
        LIBPATH=[
            dsp_lib_path,
        ]
    )