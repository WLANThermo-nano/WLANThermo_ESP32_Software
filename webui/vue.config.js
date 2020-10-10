const BundleAnalyzerPlugin = require('webpack-bundle-analyzer').BundleAnalyzerPlugin;
const CompressionPlugin = require('compression-webpack-plugin');


module.exports = {
    outputDir: `dist/${process.env.VUE_APP_PRODUCT_NAME}`,
    chainWebpack: config => {
        const svgRule = config.module.rule('svg')
        svgRule.uses.clear()

        svgRule
            .use('url-loader')
                .loader('url-loader')

        const fontsRule = config.module.rule('fonts')
        fontsRule.uses.clear()

        config.module
            .rule('fonts')
            .test(/\.(ttf|otf|eot|woff|woff2)$/)
            .use('base64-inline-loader')
            .loader('base64-inline-loader')
            .tap(options => {
                // modify the options...    
                return options
                })
            .end()

        // see here, otherwise the css and js are inlined twice
        // https://github.com/DustinJackson/html-webpack-inline-source-plugin/issues/50
        config.plugin('preload')
            .tap(args => {
                args[0].fileBlacklist.push(/\.css/, /app\.js/)
                return args
            })
        config.plugin('inline-source')
            .use(require('html-webpack-inline-source-plugin'))
        config
            .plugin('html')
            .tap(args => {
                args[0].inlineSource = '.(js|css)$'
                return args
            })
    },
    filenameHashing: false,
    configureWebpack: {
        optimization: {
            splitChunks: false
        },
        plugins: [
            // new BundleAnalyzerPlugin(),
            new CompressionPlugin()
        ]
    },
    css: {
        extract: false
    },
};