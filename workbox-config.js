module.exports = {
	maximumFileSizeToCacheInBytes: 10000000000,
	globDirectory: './',
	globPatterns: [
		'ucblogo.html',
		'ucblogo.js',
		'ucblogo.wasm',
	],
	swDest: 'sw.js',
	ignoreURLParametersMatching: [
		/^utm_/,
		/^fbclid$/
	],
	navigateFallback: '/ucblogo-code/ucblogo.html',
	runtimeCaching: [
		{
			handler: 'NetworkFirst',
			urlPattern: /.*/
		}
	],
	modifyURLPrefix: {
		'': '/ucblogo-code/',
	}
};