import { Component, ElementRef, OnInit, QueryList, ViewChild, ViewChildren } from '@angular/core';
import { TranslateService } from '@ngx-translate/core';
import { MessageService } from 'primeng/api';
import { Table, TableLazyLoadEvent } from 'primeng/table';
import { ScheduleLink, SchedulerSummary } from 'src/app/schedule/schedule.component';
import { DataService } from 'src/app/services/data.service';
import { DvrService } from 'src/app/services/dvr.service';
import { GetUpcomingRequest } from 'src/app/services/interfaces/dvr.interface';
import { ScheduleOrProgram } from 'src/app/services/interfaces/program.interface';
import { RecRule } from 'src/app/services/interfaces/recording.interface';
import { UtilityService } from 'src/app/services/utility.service';

interface RuleListEntry {
  Id: number;
  Title: string;
}


@Component({
  selector: 'app-upcoming',
  templateUrl: './upcoming.component.html',
  styleUrls: ['./upcoming.component.css'],
  providers: [MessageService]
})
export class UpcomingComponent implements OnInit, SchedulerSummary {

  @ViewChild('table') table!: Table;
  @ViewChildren('row') rows!: QueryList<ElementRef>;

  programs: ScheduleOrProgram[] = [];
  recRules: RuleListEntry[] = [];
  allRecRules: RuleListEntry[] = [];
  activeRecRules: RuleListEntry[] = [];
  defaultRecRule: RuleListEntry = { Id: 0, Title: 'settings.chanedit.all' };
  editingProgram?: ScheduleOrProgram;
  displayUpdateDlg = false;
  refreshing = false;
  loaded = false;
  inter: ScheduleLink = { summaryComponent: this };
  lazyLoadEvent!: TableLazyLoadEvent;

  displayStop = false;
  errorCount = 0;
  program?: ScheduleOrProgram;
  totalRecords = 0;
  showTable = false;
  virtualScrollItemSize = 0;
  selectedRule: RuleListEntry | null = null;
  selectedStatus = '';

  constructor(private dvrService: DvrService, private messageService: MessageService,
    private translate: TranslateService, public dataService: DataService,
    private utility: UtilityService) {
    this.translate.get(this.defaultRecRule.Title).subscribe(data => this.defaultRecRule.Title = data);
    this.loadRecRules();
  }

  ngOnInit(): void {
    // Initial Load
    this.loadLazy({ first: 0, rows: 1 });
  }

  refresh() {
    this.refreshing = true;
    this.loadRecRules();
    this.loadLazy(this.lazyLoadEvent);
  }

  loadRecRules() {
    this.dvrService.GetRecordScheduleList({}).subscribe({
      next: (data) => {
        this.allRecRules.length = 0;
        this.allRecRules.push(this.defaultRecRule);
        this.activeRecRules.length = 0;
        this.activeRecRules.push(this.defaultRecRule);
        data.RecRuleList.RecRules.forEach((entry) => {
          if (entry.Type != 'Recording Template') {
            let recRule = {
              Id: entry.Id,
              Title: entry.Title.substring(0, 30) + ' [' + this.utility.recTypeTrans[entry.Type] + ']'
            };
            this.allRecRules.push(recRule);
            if (entry.NextRecording) {
              this.activeRecRules.push(recRule);
            };
          }
        });
        this.recRules.length = 0;
        if (this.selectedStatus == 'All')
          this.recRules.push(...this.allRecRules)
        else
          this.recRules.push(...this.activeRecRules)
      },
    });
  }

  loadLazy(event: TableLazyLoadEvent) {
    this.lazyLoadEvent = event;

    let request: GetUpcomingRequest = {
      StartIndex: 0,
      Count: 1,
      ShowAll: false
    };
    if (event.first != undefined) {
      request.StartIndex = event.first;
      if (event.last)
        request.Count = event.last - event.first;
      else if (event.rows)
        request.Count = event.rows;
    }
    let sortField = '';
    if (Array.isArray(event.sortField))
      sortField = event.sortField[0];
    else if (event.sortField)
      sortField = event.sortField;
    if (!sortField)
      sortField = 'StartTime';
    if (sortField == 'Channel.ChanNum')
      request.Sort = 'ChanNum';
    else
      request.Sort = sortField;
    let sortOrder = '';
    if (event.sortOrder && event.sortOrder < 0)
      sortOrder = ' desc';
    request.Sort = request.Sort + sortOrder;

    if (this.selectedStatus == 'All')
      request.ShowAll = true;
    else if (this.selectedStatus && this.selectedStatus != 'Default')
      request.RecStatus = this.selectedStatus;
    if (this.selectedRule != null && this.selectedRule.Id != 0)
      request.RecordId = this.selectedRule.Id;
    this.recRules.length = 0;
    if (request.ShowAll)
      this.recRules.push(...this.allRecRules)
    else
      this.recRules.push(...this.activeRecRules)
    this.dvrService.GetUpcomingList(request).subscribe(data => {
      let recordings = data.ProgramList;
      this.totalRecords = data.ProgramList.TotalAvailable;
      this.programs.length = this.totalRecords;
      // populate page of virtual programs
      // note that Count is returned as the count requested, even
      // if less items are returned because you hit the end.
      // Maybe we should use recordings.Programs.length
      this.programs.splice(recordings.StartIndex, recordings.Count,
        ...recordings.Programs);
      // notify of change
      this.programs = [...this.programs]
      this.refreshing = false;
      this.showTable = true;
      let row = this.rows.get(0);
      if (row && row.nativeElement.offsetHeight)
        this.virtualScrollItemSize = row.nativeElement.offsetHeight;
      if (this.table) {
        this.table.totalRecords = this.totalRecords;
        this.table.virtualScrollItemSize = this.virtualScrollItemSize;
      }
    });

  }

  onFilter() {
    this.reload();
  }

  reload() {
    this.showTable = false;
    this.programs.length = 0;
    this.refreshing = true;
    this.loadLazy(({ first: 0, rows: 1 }));
  }

  formatStartDate(program: ScheduleOrProgram, rowIndex: number): string {
    let priorDate = '';
    if (rowIndex > 0 && this.programs[rowIndex - 1]
      && this.programs[rowIndex - 1].Recording.StartTs)
      priorDate = this.utility.formatDate(this.programs[rowIndex - 1].Recording.StartTs, true, true);
    let thisDate = this.utility.formatDate(program.Recording.StartTs, true, true);
    if (priorDate == thisDate)
      return ' ';
    return thisDate;
  }

  formatAirDate(program: ScheduleOrProgram): string {
    if (!program.Airdate)
      return ' ';
    let date = program.Airdate + ' 00:00';
    return this.utility.formatDate(date, true);
  }

  formatStartTime(program: ScheduleOrProgram): string {
    const tWithSecs = new Date(program.Recording.StartTs).toLocaleTimeString() + ' ';
    return tWithSecs.replace(/:.. /, ' ');
  }

  getDuration(program: ScheduleOrProgram): number {
    let starttm = new Date(program.Recording.StartTs).getTime();
    let endtm = new Date(program.Recording.EndTs).getTime();
    const duration = (endtm - starttm) / 60000;
    return duration;
  }

  updateRecRule(program: ScheduleOrProgram) {
    if (this.inter.sched)
      this.inter.sched.open(program);
  }

  override(program: ScheduleOrProgram) {
    if (this.inter.sched) {
      if (program.Recording.RecType == 7 || program.Recording.RecType == 8
        || program.Recording.StatusName == 'NeverRecord') // If already an override
        this.inter.sched.open(program);
      else
        this.inter.sched.open(program, undefined, <RecRule>{ Type: 'Override Recording' });
    }
  }

  stopRequest(program: ScheduleOrProgram) {
    if (program.Recording.RecordId) {
      this.program = program;
      this.displayStop = true;
    }
  }

  stopRecording(program: ScheduleOrProgram) {
    this.errorCount = 0;
    this.dvrService.StopRecording(program.Recording.RecordedId)
      .subscribe({
        next: (x) => {
          if (x.bool) {
            this.displayStop = false;
            setTimeout(() => this.inter.summaryComponent.refresh(), 3000);
          }
          else
            this.errorCount++;
        },
        error: (err) => {
          this.errorCount++;
        }
      });
  }

}
