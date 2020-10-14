<template>
  <div class="pure-g">
    <div class="pure-u-1">
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
    this.id = `chart_${Math.random()}`
  },
  mounted: function () {
    setTimeout(() => {
      this.requestDataAndUpdateChart()
      this.intervalHandle = setInterval(() => {
        this.requestDataAndUpdateChart()
      }, this.settings.iot.CLint * 1000)
    }, 50)
  },
  destroyed: function() {
    clearInterval(this.intervalHandle)
  },
  methods: {
    updateChart: function(newDatasets) {
      this.chart.data.datasets.forEach((dataset) => {
        const newData = newDatasets.find(d => d.customData.name === dataset.customData.name)
        dataset.data = newData.data
      })
      this.chart.update()
    },
    prepareChartDatasets: function(data) {
        const datasetsMap = {};
        data.cloud.data.forEach((d) => {
          const time = DateHelper.toTimestamp(d.system.time)

          d.channel.forEach((channelData) => {
            if (!datasetsMap[channelData.number]) {
              const channel = this.channels.find((c) => c.number === channelData.number)
              datasetsMap[channelData.number] = {
                yAxisID: 'y_temperature',
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
            if ( pitmasterData.typ !== 'off' ) {
              if (!datasetsMap[`pm_set_${index}`]) {
                // init pm set
                datasetsMap[`pm_set_${index}`] = {
                  yAxisID: 'y_temperature',
                  radius: 0,
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
                  yAxisID: 'y_percent',
                  radius: 0,
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
            yAxisID: 'y_percent',
            label: this.$t('battery'), 
            radius: 0,
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
                x: DateHelper.toTimestamp(d.system.time),
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
        Chart.defaults.elements.point.hitRadius = 10
        Chart.defaults.font.color= '#fff';
        /*eslint-enable */
        // eslint-disable-next-line
        this.chart = new Chart(ctx, {
          type: "line",
          data: {
            datasets: datasets,
          },
          options: {
            animation: false,
            parsing: false,
            hover: {
              mode: 'index',
              intersect: false
            },
            tooltips: {
              mode: 'index',
              intersect: false,
              filter: (tooltipItem) => {
                return this.types[tooltipItem.datasetIndex] !== 'pitmaster'
              },
              callbacks: {
                label: (tooltipItems) => {
                  return `${tooltipItems.dataPoint.y} ${this.units[tooltipItems.datasetIndex]} - ${tooltipItems.dataset.label} ${this.labelNames[tooltipItems.datasetIndex]}`;
                },
              },
            },
            point: {
                radius: 0 // default to disabled in all datasets
            },
            animation: false,
            normalized: true,
            parsing: false,
            spanGaps: true,
            elements: {
              line: {
                tension: 0, // disables bezier curves
                fill: false,
                stepped: false,
                borderDash: []
              }
            },
            aspectRatio: 2.5,
            legend: {
              labels: {
                fontColor: "#fff",
              },
            },
            scales: {
              x: {
                  type: "time",
                  ticks: {
                    autoSkip: true,
                    maxTicksLimit: 10,
                    maxRotation: 0,
                    minRotation: 0,
                  },
              },
              y_temperature: {
                position: "right",
                ticks: {
                  // eslint-disable-next-line
                  callback: (label, index, labels) => {
                    return `${label.toFixed(1)}° ${this.settings.system.unit}`
                  },
                },
              },
              y_percent: {
                position: "left",
                min: 0,
                max: 100,
                ticks: {
                  // eslint-disable-next-line
                  callback: (label, index, labels) => {
                    return label + "%"
                  },
                },
              },
            }
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
