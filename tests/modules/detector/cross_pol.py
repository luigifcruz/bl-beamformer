import time
from random import random
import blade as bl
import numpy as np

class Test(bl.Pipeline):
    detector: bl.Detector

    def __init__(self, input_shape, detector_config: bl.Detector.Config):
        bl.Pipeline.__init__(self)
        self.input = bl.cuda.cf32.ArrayTensor(input_shape)
        _config = detector_config
        _input = bl.Detector.Input(self.input)
        self.detector = self.connect(_config, _input)

    def run(self, input: bl.cpu.cf32,
                  output: bl.cpu.f32):
        self.copy(self.detector.input(), input)
        self.compute()
        self.copy(output, self.detector.output())
        self.synchronize()


if __name__ == "__main__":
    NTIME = 8750
    TFACT = 10
    NCHANS = 192
    OUTPOLS = 4
    NBEAMS = 2
    NPOLS = 2

    input_shape = (NBEAMS, NCHANS, NTIME, NPOLS)
    output_shape = (NBEAMS, NCHANS, NTIME // TFACT, OUTPOLS)

    #
    # Blade Implementation
    #

    detector_config = bl.Detector.Config(
        integration_size = TFACT,
        number_of_output_polarizations = OUTPOLS,
        
        block_size = 512
    )

    mod = Test(input_shape, detector_config)

    bl_input_raw = bl.cpu.cf32.ArrayTensor(input_shape)
    bl_output_raw = bl.cpu.f32.ArrayTensor(output_shape)

    bl_input = bl_input_raw.asnumpy()
    bl_output = bl_output_raw.asnumpy()
    
    np.copyto(bl_input, np.random.random(size=bl_input.shape) + 1j*np.random.random(size=bl_input.shape))

    start = time.time()
    mod.run(bl_input_raw, bl_output_raw)
    print(f"Detection with Blade took {time.time()-start:.2f} s.")

    #
    # Python Implementation
    #

    py_output = np.zeros(output_shape, dtype=np.float32)
    
    start = time.time()
    for ibeam in range(NBEAMS):
        for ichan in range(NCHANS):
            for isamp in range(NTIME//TFACT):
                x = bl_input[ibeam, ichan, isamp*TFACT:isamp*TFACT+TFACT, 0] #just to make code more visible
                y = bl_input[ibeam, ichan, isamp*TFACT:isamp*TFACT+TFACT, 1] #just to make code more visible

                auto_x = x*np.conj(x) # or: x.real*x.real + x.imag*x.imag... this will definitely be a real value, no .imag part
                auto_y = y*np.conj(y) # or: y.real*y.real + y.imag*y.imag... this will definitely be a real value, no .imag part
                cross  = x*np.conj(y) # or:

                py_output[ibeam, ichan, isamp, 0] = np.sum(auto_x.real) #real is actually abs() too, because x*xT
                py_output[ibeam, ichan, isamp, 1] = np.sum(auto_y.real) #real is actually abs() too, because y*yT
                py_output[ibeam, ichan, isamp, 2] = np.sum(cross.real)
                py_output[ibeam, ichan, isamp, 3] = np.sum(cross.imag)
    print(f"Detection with Python took {time.time()-start:.2f} s.")

    #
    # Compare Results
    #

    assert np.allclose(bl_output, py_output, rtol=0.1)
    print("Test successfully completed!")
