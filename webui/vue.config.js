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

        // Inline all JS/CSS into the HTML file — only for production builds.
        // In dev mode the scripts land in <head> before #app exists → mount fails.
        // inject:'body' ensures the inline script is placed after <div id="app">.
        if (process.env.NODE_ENV === 'production') {
            config.plugin('inline-source')
                .use(require('@effortlessmotion/html-webpack-inline-source-plugin'))
            config
                .plugin('html')
                .tap(args => {
                    args[0].inlineSource = '.(js|css)$'
                    args[0].inject = 'body'
                    return args
                })
        }
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
        proxy: {
            '/': {
                target: 'http://localhost',
                ws: false,  // don't proxy HMR websocket (/ws) to the device
                bypass: function(req) {
                    // Vue Router routes (browser navigation) must not be proxied —
                    // return index.html so the SPA handles routing client-side.
                    if (req.headers.accept && req.headers.accept.includes('text/html')) {
                        return '/index.html'
                    }
                }
            }
        }
    }
};
