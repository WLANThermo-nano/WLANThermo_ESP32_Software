module.exports = {
  purge: ["./src/**/*.{vue}"],
  darkMode: false, // or 'media' or 'class'
  theme: {
    extend: {
    },
    colors: {
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
        500: "#a9adb0",
        800: "#7c7c7c"
      }
    }
  },
  variants: {
    extend: {
      backgroundColor: ['active'],
      opacity: ['disabled'],
      cursor: ['disabled']
    },
  },
  plugins: [],
}
