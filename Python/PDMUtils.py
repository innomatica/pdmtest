#!/usr/bin/env python3

import numpy as np


def GeneratePcmSine(amp, freq, srate, ncycle):

    # total duration: ncycle * 1 / freq
    # total data count = duration (ncycle/freq) times sample rate
    count = int(srate * ncycle / freq + 0.5)
    # generate time series with eqidistant interval in the cycle
    t = np.linspace(0, 2 * np.pi * ncycle , count, False)

    return t, (amp * np.sin(t)).astype(np.int16)


def GeneratePdmSine(freq, srate, ncycle):

    # total duration: ncycle * 1 / freq
    # total data count = duration (ncycle/freq) times sample rate
    count = int(srate * ncycle / freq + 0.5)
    # generate time series with eqidistant interval in the cycle
    t = np.linspace(0, 2 * np.pi * ncycle , count, False)
    # PCM reference
    r = np.sin(t)
    # PDM data for plotting
    y = np.zeros_like(r)
    # packed PDM data
    p = np.zeros(int((len(y)/16 + 0.5)) + 0, np.uint16)
    # initial values
    prv_d = 0
    k = 0
    # assume MSB first
    bit_pos = 15

    for i in range(1, len(r)):
        # difference
        d = r[i-1] - y[i-1]
        # integration 1st order
        int_d = d + prv_d
        # 1 bit quantization
        if(int_d > 0):
            y[i] = 1
            p[k] = p[k] + (1<<bit_pos)

        else:
            y[i] = -1

        # iteration
        prv_d = int_d
        bit_pos = bit_pos - 1

        if(bit_pos == -1):
            bit_pos = 15
            k = k + 1

    # time, float (-1 or 1), 16bit packet
    return t, y, p

def FilterCIC(indata, order = 5, decim = 16):
    '''
    CIC filter

    order: 3, 4, or 5
    decim: 16
    '''

    # in decimation 16, indata and outdata are equal in size
    outdata = np.zeros((indata.size), dtype=np.int)

    # initialial values
    del1 = del2 = del3 = del4 = del5 = delx = 0
    sig1 = sig2 = sig3 = sig4 = sig5 = sigx = 0

    pdel1 = pdel2 = pdel3 = pdel4 = 0
    psigx = 0

    for i in range(indata.size):
        # iteration
        psigx = sigx

        pdel1 = del1
        pdel2 = del2
        pdel3 = del3
        pdel4 = del4

        # integrator section with decimator
        for j in range(16):
            if ((indata[i] >> (15-j)) & 0x01) == 0x01:
                sig1 += 1
            else:
                sig1 -= 1

            sig2 += sig1
            sig3 += sig2
            sig4 += sig3
            sig5 += sig4

            if order == 2:
                sigx = sig2
            elif order == 3:
                sigx = sig3
            elif order == 4:
                sigx = sig4
            else:
                sigx = sig5

        # comb section
        del1 = sigx - psigx
        del2 = del1 - pdel1
        del3 = del2 - pdel2
        del4 = del3 - pdel3
        del5 = del4 - pdel4

        if order == 2:
            delx = del2
        elif order == 3:
            delx = del3
        elif order == 4:
            delx = del4
        else:
            delx = del5

        outdata[i] = delx
        '''
        print('sig1:{}, sig5:{},indata:{:04x}, outdata:{:04x}'.format(
            sig1, sig5, indata[i], outdata[i]))
        '''
    return outdata

def FilterHalfBand():
    pass

def FilterFIR():
    pass

#-------------------------------------------------------------------------------
if __name__=='__main__':

    # import pyplot
    import matplotlib.pyplot as plt

    #---------------------------------------------------------------------------
    # PCM sinewave data at 16KHz sampling
    x, y = GeneratePcmSine(30000., 1500, 16000, 10)

    # print out
    print("\r\nPCM Sine data...................................\r\n")
    for i in range(len(y)):
        #print('[{:02d}] {:05d}'.format(i, y[i]))
        print('[{:02d}] {}'.format(i, y[i]))

    # time plot
    fig = plt.figure()
    fig.suptitle('PCM Signal')
    sub = plt.subplot(2,1,1)
    sub.set_title('Time domain')
    #sub.plot(x,y)
    sub.plot(y, antialiased=False)

    delta = 1/16000.

    # fft
    sp = np.fft.rfft(y) * delta * 2
    freq = np.fft.rfftfreq(y.size, delta)

    sub = plt.subplot(2,1,2)
    sub.set_title('FFT')
    sub.plot(freq, np.abs(sp) )
    #plt.show()

    #---------------------------------------------------------------------------
    # PDM sinewave data 64 times oversampling of 16kHz
    x, y, p = GeneratePdmSine(1000, 16000 * 64, 10)

    # print out
    '''
    for i in range(len(y)):
        print('[{:02d}] {}'.format(i, y[i]))
    '''

    print("\r\nPDM Sine data...................................\r\n")
    for i in range(len(p)):
        # bit format
        #print('[{:02d}] - {:016b}'.format(i, p[i]))
        # word format
        print('[{:02d}] 0x{:04x}'.format(i, p[i]))

    # time plot
    fig = plt.figure()
    fig.suptitle('PDM Signal')
    sub = plt.subplot(2,1,1)
    sub.set_title('Time domain')
    sub.plot(x,y)

    delta = 1/16000/64

    # fft: this does not have much meaning but the noise shape
    sp = np.fft.rfft(y) * delta * 2
    freq = np.fft.rfftfreq(y.size, delta)

    sub = plt.subplot(2,1,2)
    sub.plot(freq, np.abs(sp) )
    sub.set_title('FFT')
    #plt.show()

    #---------------------------------------------------------------------------
    d1 = FilterCIC(p, 3)

    print("\r\nPDM CIC filtered................................\r\n")
    for i in range(len(d1)):
        # bit format
        #print('[{:02d}] - {:016b}'.format(i, p[i]))
        # word format
        print('[{:02d}]  {}'.format(i, d1[i]))

    # time plot
    fig = plt.figure()
    fig.suptitle('CIC filter applied')
    sub = plt.subplot(2,1,1)
    sub.set_title('Time domain')
    sub.plot(d1, antialiased=False)

    delta = 1/16000/4

    # fft
    sp = np.fft.rfft(d1) * delta * 2
    freq = np.fft.rfftfreq(d1.size, delta)

    sub = plt.subplot(2,1,2)
    sub.plot(freq, np.abs(sp) )
    sub.set_title('FFT')
    plt.show()


