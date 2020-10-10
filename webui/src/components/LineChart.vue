<template>
  <div class="pure-g">
    <div class="pure-u-1">
      <button @click="requestDataAndUpdateChart"> Update (dev only) </button>
      <div class="chart-container">
        <canvas :id="id"></canvas>
      </div>
    </div>
  </div>
</template>

<script>
import DateHelper from '../helpers/date-helper'

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
      chart: null,
      canUseChart: false,
      labelNames: [],
      units: [],
      types: [],
      intervalHandle: null
    };
  },
  created: function () {
    // eslint-disable-next-line
    this.canUseChart = typeof Chart === "function"
    this.id = `chart_${Math.random()}`
  },
  mounted: function () {
    if (this.canUseChart) {
      this.requestDataAndUpdateChart()
      this.intervalHandle = setInterval(() => {
        this.requestDataAndUpdateChart()
      }, this.settings.iot.CLint * 1000)
    }
  },
  destroyed: function() {
    clearInterval(this.intervalHandle)
  },
  methods: {
    updateChart: function(datasets) {
      this.chart.data.labels.pop();
      this.chart.data.datasets = []
      datasets.forEach(dataset => {
        this.chart.data.datasets.push(dataset)
      })
      this.chart.update({
        duration: 0,
      })
    },
    prepareChartDatasets: function(data) {
        const datasetsMap = {};
        data.cloud.data.forEach((d) => {
          const time = DateHelper.stringTimestampInSecondToDate(d.system.time)

          d.channel.forEach((channelData) => {
            if (!datasetsMap[channelData.number]) {
              const channel = this.channels.find((c) => c.number === channelData.number)
              datasetsMap[channelData.number] = {
                yAxisID: 'A',
                label: `#${channelData.number}`,
                borderColor: channel.color,
                backgroundColor: channel.color,
                fill: false,
                customData: {
                  name: channel.name,
                  unit: this.settings.system.unit,
                  type: 'channel'
                },
                data: [],
              };
            }
            datasetsMap[channelData.number].data.push({
              x: time,
              y: channelData.temp,
            });
          });

          d.pitmaster.forEach((pitmasterData, index) => {
            if ( pitmasterData.type !== 'off' ) {
              if (!datasetsMap[`pm_set_${index}`]) {
                // init pm set
                datasetsMap[`pm_set_${index}`] = {
                  yAxisID: 'A',
                  radius: 0,
                  lineTension: 0,
                  borderColor: '#FF0000',
                  borderDash: [10,5],
                  label: `pm set #${index + 1}`,
                  customData: {
                    name: `pm set #${index + 1}`,
                    unit: this.settings.system.unit,
                    type: 'pitmaster'
                  },
                  data: []
                }
                // init pm value
                datasetsMap[`pm_val_${index}`] = {
                  yAxisID: 'B',
                  radius: 0,
                  lineTension: 0,
                  borderColor: '#000000',
                  borderWidth: 1,
                  label: `pm val #${index + 1}`,
                  customData: {
                    name: `pm val #${index + 1}`,
                    unit: `%`,
                    type: 'pitmaster'
                  },
                  data: []
                }
              }

              datasetsMap[`pm_set_${index}`].data.push({
                x: time,
                y: pitmasterData.set,
              });
              datasetsMap[`pm_val_${index}`].data.push({
                x: time,
                y: pitmasterData.value,
              });
            }
          })
        });

        if (data.cloud.data.some(d => d.system.soc)) {
          datasetsMap[`battery`] = {
            yAxisID: 'B',
            label: this.$t('battery'), 
            radius: 0,
            lineTension: 0,
            borderColor: '#fff',
            backgroundColor: '#fff',
            fill: false,
            borderWidth: 1,
            customData: {
              name: this.$t('battery'),
              unit: `%`,
              type: 'battery'
            },
            data: data.cloud.data.filter(d => d.system.soc).map(d => {
              return {
                x: DateHelper.stringTimestampInSecondToDate(d.system.time),
                y: d.system.soc
              }
            })
          }
        }

        const datasets = Object.keys(datasetsMap).map((k) => datasetsMap[k]);
        this.labelNames = datasets.map(d => d.customData.name)
        this.units = datasets.map(d => d.customData.unit)
        this.types = datasets.map(d => d.customData.type)

        return datasets;
    },
    requestDataAndUpdateChart: function() {
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
        const datasets = this.prepareChartDatasets(response.data)
        if (!this.chart) {
          this.initChart(datasets)
        } else {
          this.updateChart(datasets)
        }
      })
    },
    initChart: function (datasets) {
        let ctx = document.getElementById(this.id).getContext("2d");
        /*eslint-disable */
        // Chart.defaults.global.defaultFontColor = "#fff";
        // eslint-disable-next-line
        // Chart.defaults.global.defaultFontSize = 15;
        Chart.defaults.global.animationSteps = 50;
        Chart.defaults.global.tooltipYPadding = 16;
        Chart.defaults.global.tooltipCornerRadius = 0;
        Chart.defaults.global.defaultFontColor= 'white';
        Chart.defaults.global.tooltipTitleFontStyle = "normal";
        Chart.defaults.global.tooltips.mode = "index";
        Chart.defaults.global.tooltips.intersect = false;
        Chart.defaults.global.legend.labels.boxWidth = 15;
        Chart.defaults.global.animationEasing = "easeOutBounce";
        Chart.defaults.global.responsive = true;
        Chart.defaults.global.scaleFontSize = 16;
        /*eslint-enable */
        // eslint-disable-next-line
        this.chart = new Chart(ctx, {
          type: "line",
          data: {
            datasets: datasets,
          },
          options: {
            aspectRatio: 2.5,
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
              filter: (tooltipItem) => {
                return this.types[tooltipItem.datasetIndex] !== 'pitmaster'
              },
              callbacks: {
                label: (tooltipItems, data) => {
                  return `${tooltipItems.yLabel} ${this.units[tooltipItems.datasetIndex]} - ${data.datasets[tooltipItems.datasetIndex].label} ${this.labelNames[tooltipItems.datasetIndex]}`;
                },
              },
            },
          },
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
    margin-left: 0;
    width: 100vw;
  }
}
</style>
