
// define the service that grabs the cluster info
(function (angular) {
    'use strict';

    angular.module('app.clusterInfo', [])
        .factory('clusterInfo', ['$http', function ($http) {
            return {
                get: function () {
                    return $http({
                        method: 'GET',
                        url: 'api/cluster-info'
                    });
                }
            };
        }]);

}(angular));