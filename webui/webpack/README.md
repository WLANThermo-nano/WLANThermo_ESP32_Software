## About inline chunk html plugin

It's a webpack plugin which inject js file in to html file. The plugin is made by Facebook and copied from [Facebook/create-react-app](https://github.com/facebook/create-react-app/blob/master/packages/react-dev-utils/InlineChunkHtmlPlugin.js)


The plugin takes two arguments HtmlWebpackPlugin and a list of file name which you want to inject.

```
module.exports = {
  plugins: [
    new HtmlWebpackPlugin({
      'public/index.html'
    }),
    // the second argument is the file name you want to inject
    new InlineChunkHtmlPlugin(HtmlWebpackPlugin, ['/js/app.js']),
  ],
};
```