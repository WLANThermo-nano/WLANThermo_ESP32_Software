const BundleAnalyzerPlugin = require('webpack-bundle-analyzer').BundleAnalyzerPlugin;
const CompressionPlugin = require('compression-webpack-plugin');

module.exports = {
    filenameHashing: false,
    configureWebpack: {
        plugins: [
            new CompressionPlugin(),
            new BundleAnalyzerPlugin()
        ],
    },
    devServer: {
        proxy: 'http://localhost'
    }
};