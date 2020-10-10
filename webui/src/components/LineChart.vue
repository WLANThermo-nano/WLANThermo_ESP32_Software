<template>
  <div class="pure-u-1-1 pure-u-lg-1-1">
    <div class="chart-container">
      <canvas :id="id"></canvas>
    </div>
  </div>
</template>

<script>
export default {
  name: "LineChart",
  props: {
    settings: {
      type: Object,
    },
    channels: {
      type: Array,
    },
  },
  data: () => {
    return {
      id: '',
      canUseChart: false,
      labelNames: []
    };
  },
  watch: {},
  created: function () {
    // eslint-disable-next-line
    this.canUseChart = typeof Chart === "function";
    this.id = `chart_${Math.random()}`;
  },
  mounted: function () {
    if (this.canUseChart) {
      this.initChart();
    }
  },
  methods: {
    initChart: function () {
      const requestObj = {
        device: this.settings.device,
        cloud: {
          task: "read",
          api_token: this.settings.iot.CLtoken,
        },
      };
      var config = {
        method: "post",
        url: this.settings.iot.CLurl, // https://dev-api.wlanthermo.de/index.php
        headers: {
          "Content-Type": "text/plain",
        },
        data: requestObj,
      };

      this.axios(config).then((response) => {
        const data = response.data;

        // data structure in data.cloud.data
        // {
        //     "system": {
        //         "time": "1602327464",
        //         "soc": 84
        //     },
        //     "channel": [
        //         {
        //             "number": 1,
        //             "temp": 110.9
        //         },
        //         {
        //             "number": 2,
        //             "temp": 56.7
        //         }
        //     ],
        //     "pitmaster": [
        //         {
        //             "value": 0,
        //             "set": 50,
        //             "typ": "off"
        //         }
        //     ]
        // },
        const datasetsMap = {};
        data.cloud.data.forEach((d) => {
          d.channel.forEach((channelData) => {
            if (!datasetsMap[channelData.number]) {
              const channel = this.channels.find((c) => c.number === channelData.number)
              datasetsMap[channelData.number] = {
                label: `#${channelData.number}`,
                borderColor: channel.color,
                backgroundColor: channel.color.color,
                fill: false,
                customData: {
                  name: channel.name
                },
                data: [],
              };
            }
            datasetsMap[channelData.number].data.push({
              x: new Date(parseInt(d.system.time * 1000)),
              y: channelData.temp,
            });
          });

          d.pitmaster.forEach(() => {
            if (!datasetsMap['pm_set'] && !datasetsMap['pm_val']) {
              datasetsMap['pm_set'] = {
                radius: 0,
                lineTension: 0,
                borderColor: '#FF0000',
                borderDash: [10,5],
                label: 'pm set',
                customData: {
                  name: 'pm set'
                },
              }
            }
          })
        });

        const datasets = Object.keys(datasetsMap).map((k) => datasetsMap[k]);
        this.labelNames = datasets.map(d => d.customData.name)

        let ctx = document.getElementById(this.id).getContext("2d");
        // eslint-disable-next-line
        Chart.defaults.global.defaultFontColor = "#fff";
        // eslint-disable-next-line
        Chart.defaults.global.defaultFontSize = 15;
        // eslint-disable-next-line
        new Chart(ctx, {
          type: "line",
          data: {
            datasets: datasets,
          },
          options: {
            legend: {
              labels: {
                fontColor: "#fff",
              },
            },
            scales: {
              xAxes: [
                {
                  type: "time",
                  ticks: {
                    autoSkip: true,
                    maxTicksLimit: 10,
                    maxRotation: 0,
                    minRotation: 0,
                  },
                },
              ],

              yAxes: [
                {
                  id: "A",
                  position: "right",
                  ticks: {
                    // eslint-disable-next-line
                    userCallback: (label, index, labels) => {
                      return `${label.toFixed(1)}° ${this.settings.system.unit}`
                    },
                  },
                },
                {
                  id: "B",
                  position: "left",
                  ticks: {
                    min: 0,
                    max: 100,
                    // eslint-disable-next-line
                    userCallback: (label, index, labels) => {
                      return label + "%"
                    },
                  },
                },
              ],
            },

            tooltips: {
              mode: 'index',
              enabled: true,
              callbacks: {
                label: (tooltipItems, data) => {
                  return `${tooltipItems.yLabel} ${this.settings.system.unit} - ${data.datasets[tooltipItems.datasetIndex].label} ${this.labelNames[tooltipItems.datasetIndex]}`;
                },
              },
            },
          },
        });
      });
    },
  },
  components: {},
};
</script>

<style lang="scss" scoped>
@import "../assets/global.scss";

.chart-container {
  margin-left: 5px;
  background-color: $medium_dark;
  position: relative;
  // height:40vh;
  position: relative;
  width: calc(100vw - #{$nav_width} - 10px);
  @media screen and (max-width: $mobile_break_point) {
    width: 100vw;
  }
}
</style>
