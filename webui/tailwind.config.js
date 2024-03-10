module.exports = {
  purge: [
    './public/**/*.html',
    './src/**/*.vue',
    './src/**/*.js',
  ],
  darkMode: 'class', // or 'media' or 'class'
  theme: {
    extend: {
      
      backgroundImage: {
        'light-temp-up': "url('./icons/svg/light-temp-up.svg')",
        'light-temp-down': "url('./icons/svg/light-temp-down.svg')",

        'temp-up': "url('./icons/svg/temp-up.svg')",
        'temp-down': "url('./icons/svg/temp-down.svg')",

        'light-wifi-10': "url('./icons/svg/light-wifi-10.svg')",
        'light-wifi-50': "url('./icons/svg/light-wifi-50.svg')",
        'light-wifi-100': "url('./icons/svg/light-wifi-100.svg')",

        'wifi-10': "url('./icons/svg/wifi-10.svg')",
        'wifi-50': "url('./icons/svg/wifi-50.svg')",
        'wifi-100': "url('./icons/svg/wifi-100.svg')",

        'light-charge-10': "url('./icons/svg/light-charge-10.svg')",
        'light-charge-25': "url('./icons/svg/light-charge-25.svg')",
        'light-charge-50': "url('./icons/svg/light-charge-50.svg')",
        'light-charge-75': "url('./icons/svg/light-charge-75.svg')",
        'light-charge-100': "url('./icons/svg/light-charge-100.svg')",
        'light-battery-10': "url('./icons/svg/light-battery-10.svg')",
        'light-battery-25': "url('./icons/svg/light-battery-25.svg')",
        'light-battery-50': "url('./icons/svg/light-battery-50.svg')",
        'light-battery-75': "url('./icons/svg/light-battery-75.svg')",
        'light-battery-100': "url('./icons/svg/light-battery-100.svg')",

        'charge-10': "url('./icons/svg/charge-10.svg')",
        'charge-25': "url('./icons/svg/charge-25.svg')",
        'charge-50': "url('./icons/svg/charge-50.svg')",
        'charge-75': "url('./icons/svg/charge-75.svg')",
        'charge-100': "url('./icons/svg/charge-100.svg')",
        'battery-10': "url('./icons/svg/battery-10.svg')",
        'battery-25': "url('./icons/svg/battery-25.svg')",
        'battery-50': "url('./icons/svg/battery-50.svg')",
        'battery-75': "url('./icons/svg/battery-75.svg')",
        'battery-100': "url('./icons/svg/battery-100.svg')",
      },
      minWidth: {
        120: '120px',
        160: '160px',
        180: '180px'
      },
      minHeight: {
        400: '400px'
      }
    },
    colors: {
      // light
      yellow: {
        600: '#FFD524'
      },
      pink: {
        600: '#FF007C'
      },
      lightblue: {
        600: '#F7F9FF',
        700: '#e3eafc',
        800: '#2D62ED'
      },
      blue: {
        800: '#272D3B'
      },
      // dark
      primary: {
        200: "#9ef7ff",
        400: "#00D7FF", // base
        600: "#0095c0"
      },
      white: "#fff",
      darkblue: {
        600: "#455060",
        700: "#333c4a",
        800: "#29313A",
        900: "#181D23"
      },
      error: "#ff007c",
      red: '#FF0000',
      grey: {
        100: '#F3F4F6',
        500: "#a9adb0",
        800: "#7c7c7c"
      }
    }
  },
  variants: {
    extend: {
      backgroundColor: ['active'],
      backgroundImage: ['dark'],
      opacity: ['disabled'],
      cursor: ['disabled']
    },
  },
  plugins: [],
}
