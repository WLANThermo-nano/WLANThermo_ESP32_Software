const BundleAnalyzerPlugin = require('webpack-bundle-analyzer').BundleAnalyzerPlugin;
const CompressionPlugin = require('compression-webpack-plugin');
const HtmlWebpackPlugin = require('html-webpack-plugin');
const InlineChunkHtmlPlugin = require('./webpack/inline-chunk-html-plugin');

module.exports = {
    filenameHashing: false,
    configureWebpack: {
        optimization: {
            splitChunks: false
        },
        plugins: [
            // new BundleAnalyzerPlugin(),
            new HtmlWebpackPlugin({
                template: 'public/index.html',  //template file to embed the source
            }),
            new InlineChunkHtmlPlugin(HtmlWebpackPlugin, ['js/app.js']),
            new CompressionPlugin(),
        ],
    },
    css: {
        extract: false
    },
    devServer: {
        proxy: 'http://localhost'
    }
};