import numpy as np
import matplotlib.pyplot as plt

np1_short  = [
20.054,10.038,5.124,3.525,2.636,1.678,
]
np2_short = [
20.056,10.153,5.234,3.635,2.846,1.582,
]
np4_short = [
20.073,10.17,5.472,3.75,3.044,1.719,
]
np6_short = [
20.111,10.236,5.316,3.946,3.002,1.978,
]
np8_short = [
20.206,10.326,5.397,4.048,3.054,1.794,
]
np16_short = [
20.514,10.787,5.51,3.921,3.018,1.924,
]

np1_long  = [
103.061,51.644,25.941,17.435,13.233,6.78,
]
np2_long = [
103.135,51.671,26.127,17.462,13.442,6.806,
]
np4_long = [
103.339,51.889,25.996,17.622,13.284,7.101,
]
np6_long = [
104.077,52.027,26.21,17.707,13.253,6.901,
]
np8_long = [
104.433,52.287,26.441,17.566,13.43,7.329,
]
np16_long = [
107.973,54.428,27.378,18.61,14.055,7.262,
]

nproducers = [1, 2, 4, 6, 8, 16]
fig, axs = plt.subplots(1, 2, constrained_layout=False, figsize=(15, 3))
plt.subplots_adjust(hspace=.5)

axs[0].plot(nproducers,
            np1_short,
            linestyle='-',
            #         color='red',
            label='number of producers=1'
            )
axs[0].plot(nproducers,
            np2_short,
            linestyle='-',
            #         color='blue',
            label='number of producers=2'
            )
axs[0].plot(nproducers,
            np4_short,
            linestyle='-',
            #         color='blue',
            label='number of producers=4'
            )

axs[0].plot(nproducers,
            np6_short,
            linestyle='-',
            #         color='blue',
            label='number of producers=6'
            )

axs[0].plot(nproducers,
            np8_short,
            linestyle='-',
            #         color='blue',
            label='number of producers=8'
            )
axs[0].plot(nproducers,
            np16_short,
            linestyle='-',
            #         color='blue',
            label='number of producers=16'
            )

axs[0].set_xlabel('the number of consumers', fontsize=14)
axs[0].set_ylabel('time (s)', fontsize=14)

axs[0].legend(fontsize=14)

axs[1].plot(nproducers,
            np1_long,
            linestyle='-',
            #         color='red',
            label='number of producers=1'
            )
axs[1].plot(nproducers,
            np2_long,
            linestyle='-',
            #         color='blue',
            label='number of producers=2'
            )
axs[1].plot(nproducers,
            np4_long,
            linestyle='-',
            #         color='blue',
            label='number of producers=4'
            )

axs[1].plot(nproducers,
            np6_long,
            linestyle='-',
            #         color='blue',
            label='number of producers=6'
            )

axs[1].plot(nproducers,
            np8_long,
            linestyle='-',
            #         color='blue',
            label='number of producers=8'
            )
axs[1].plot(nproducers,
            np16_long,
            linestyle='-',
            #         color='blue',
            label='number of producers=16'
            )

axs[1].set_xlabel('the number of consumers', fontsize=14)
axs[1].set_ylabel('time (s)', fontsize=14)

axs[1].legend(fontsize=14)
