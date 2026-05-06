const CompressionPlugin = require('compression-webpack-plugin');

module.exports = {
    outputDir: `dist/${process.env.VUE_APP_PRODUCT_NAME}`,
    chainWebpack: config => {
        // SVG: inline as data URI (webpack 5 asset module, replaces url-loader)
        const svgRule = config.module.rule('svg')
        svgRule.uses.clear()
        svgRule.delete('type')
        svgRule.delete('generator')
        svgRule.set('type', 'asset/inline')

        // Fonts: inline as base64 data URI (webpack 5 asset module, replaces base64-inline-loader)
        const fontsRule = config.module.rule('fonts')
        fontsRule.uses.clear()
        fontsRule.delete('type')
        fontsRule.delete('generator')
        fontsRule.test(/\.(ttf|otf|eot|woff|woff2)$/)
        fontsRule.set('type', 'asset/inline')

        // Remove preload/prefetch — everything is inlined into HTML anyway
        config.plugins.delete('preload')
        config.plugins.delete('prefetch')

        // Inline all JS/CSS into the HTML file (single-file output for firmware embedding)
        config.plugin('inline-source')
            .use(require('@effortlessmotion/html-webpack-inline-source-plugin'))
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
            // new (require('webpack-bundle-analyzer').BundleAnalyzerPlugin)(),
            new CompressionPlugin()
        ]
    },
    css: {
        extract: false
    },
    devServer: {
        proxy: 'http://localhost'
    }
};
