const BundleAnalyzerPlugin = require('webpack-bundle-analyzer').BundleAnalyzerPlugin;
const CompressionPlugin = require('compression-webpack-plugin');

module.exports = {
    filenameHashing: false,
    configureWebpack: {
        plugins: [
            new CompressionPlugin(),
            // new BundleAnalyzerPlugin()
        ],
        // output: {
        //     filename: '[name].js',
        //     chunkFilename: '[name].js'
        // }
    },
    devServer: {
        proxy: 'http://localhost'
    }
};