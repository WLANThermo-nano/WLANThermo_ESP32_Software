module.exports = {
  purge: ["./src/**/*.{vue}"],
  darkMode: false, // or 'media' or 'class'
  theme: {
    extend: {},
    colors: {
      primary: "#00D7FF",
      white: "#fff",
      darkblue: {
        600: "##455060",
        900: "#29313A"
      },
      grey: "#a9adb0"
    }
  },
  variants: {
    extend: {},
  },
  plugins: [],
}
