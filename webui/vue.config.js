const BundleAnalyzerPlugin = require('webpack-bundle-analyzer').BundleAnalyzerPlugin;
const CompressionPlugin = require('compression-webpack-plugin');
const HtmlWebpackPlugin = require('html-webpack-plugin');
const InlineChunkHtmlPlugin = require('./webpack/inline-chunk-html-plugin');

module.exports = {
    outputDir: `dist/${process.env.VUE_APP_PRODUCT_NAME}`,
    chainWebpack: config => {
        const svgRule = config.module.rule('svg')
        svgRule.uses.clear()

        svgRule
            .use('url-loader')
                .loader('url-loader')
    },
    filenameHashing: false,
    configureWebpack: {
        optimization: {
            splitChunks: false
        },
        plugins: [
            // new BundleAnalyzerPlugin(),
            new InlineChunkHtmlPlugin(HtmlWebpackPlugin, ['js/app.js']),
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